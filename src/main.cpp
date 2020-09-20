/******************************************************************************
 *                                                                            *
 * Copyright (C) 2020 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <limits>
#include <random>
#include <fstream>

#include <yarp/os/Network.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Value.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/os/RpcServer.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/GazeControl.h>
#include <yarp/sig/Vector.h>
#include <yarp/sig/Matrix.h>
#include <yarp/sig/Image.h>
#include <yarp/sig/PointCloud.h>
#include <yarp/math/Math.h>

#include "rpc_IDL.h"
#include "viewer.h"
#include "segmentation.h"
#include "cardinal_points_grasp.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;
using namespace viewer;
using namespace segmentation;
using namespace cardinal_points_grasp;

/******************************************************************************/
class GrasperModule : public RFModule, public rpc_IDL {
    PolyDriver arm_r,arm_l;
    PolyDriver hand_r,hand_l;
    PolyDriver gaze;

    vector<pair<string, shared_ptr<BufferedPort<Bottle>>>> objMoverPorts;

    RpcServer rpcPort;
    BufferedPort<ImageOf<PixelRgb>> rgbPort;
    BufferedPort<ImageOf<PixelFloat>> depthPort;
    RpcClient sqPort;

    vector<int> fingers = {7, 8, 9, 10, 11, 12, 13, 14, 15};
    shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>> pc_scene{nullptr};
    shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>> pc_table{nullptr};
    shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>> pc_object{nullptr};

    Matrix Teye;
    double view_angle{50.};
    double table_height{numeric_limits<double>::quiet_NaN()};
    Bottle sqParams;
    
    unique_ptr<Viewer> viewer;
    
    /**************************************************************************/
    bool attach(RpcServer& source) override {
        return this->yarp().attachAsServer(source);
    }

    /**************************************************************************/
    auto savePCL(const string& filename, shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>> pc) {
        ofstream fout(filename);
        if (fout.is_open()) {
            fout << "COFF" << endl;
            fout << pc->size() << " 0 0" << endl;
            for (size_t i = 0; i < pc->size(); i++) {
                const auto& p = (*pc)(i);
                fout << p.x << " " << p.y << " " << p.z << " "
                     << (int)p.r << " " << (int)p.g << " "
                     << (int)p.b << " " << (int)p.a << endl;
            }
            return true;
        } else {
            yError() << "Unable to write to" << filename;
            return false;
        }
    }

    /**************************************************************************/
    auto helperArmDriverOpener(PolyDriver& arm, const Property& options) {
        const auto t0 = Time::now();
        while (Time::now() - t0 < 10.) {
            // this might fail if controller is not connected to solver yet
            if (arm.open(const_cast<Property&>(options))) {
                return true;
            }
            Time::delay(1.);
        }
        return false;
    }

    /**************************************************************************/
    bool lookAtDeveloper() {
        IGazeControl* igaze;
        gaze.view(igaze);
        igaze->lookAtAbsAnglesSync({60., 15., 5.});
        igaze->waitMotionDone();
        return true;
    }

    /**************************************************************************/
    bool shrug() {
        Vector x{-.1, .3, .15};
        Vector o{0., 0., 1., M_PI / 2.};
        vector<PolyDriver*> polys({&arm_r, &arm_l});
        ICartesianControl* iarm;
        for (size_t i = 0; i < polys.size(); i++) {
            polys[i]->view(iarm);
            iarm->setPosePriority("orientation");
            iarm->goToPoseSync(x, o);

            x[1] = -x[1];
            o = dcm2axis(axis2dcm(o) * axis2dcm({0., 1., 0., M_PI}));
        }

        // wait only for the last arm
        iarm->waitMotionDone(.1, 3.);
        return true;
    }

    /**************************************************************************/
    bool configure(ResourceFinder& rf) override {
        const string name = "icub-grasp";

        Property arm_r_options;
        arm_r_options.put("device", "cartesiancontrollerclient");
        arm_r_options.put("local", "/"+name+"/arm_r");
        arm_r_options.put("remote", "/icubSim/cartesianController/right_arm");
        if (!helperArmDriverOpener(arm_r, arm_r_options)) {
            yError() << "Unable to open right arm driver!";
            return false;
        }

        Property arm_l_options;
        arm_l_options.put("device", "cartesiancontrollerclient");
        arm_l_options.put("local", "/"+name+"/arm_l");
        arm_l_options.put("remote", "/icubSim/cartesianController/left_arm");
        if (!helperArmDriverOpener(arm_l, arm_l_options)) {
            yError() << "Unable to open left arm driver!";
            arm_r.close();
            return false;
        }

        Property hand_r_options;
        hand_r_options.put("device", "remote_controlboard");
        hand_r_options.put("local", "/"+name+"/hand_r");
        hand_r_options.put("remote", "/icubSim/right_arm");
        if (!hand_r.open(hand_r_options)) {
            yError() << "Unable to open right hand driver!";
            arm_r.close();
            arm_l.close();
            return false;
        }

        Property hand_l_options;
        hand_l_options.put("device", "remote_controlboard");
        hand_l_options.put("local", "/"+name+"/hand_l");
        hand_l_options.put("remote", "/icubSim/left_arm");
        if (!hand_l.open(hand_l_options)) {
            yError() << "Unable to open left hand driver!";
            arm_r.close();
            arm_l.close();
            hand_r.close();
            return false;
        }

        Property gaze_options;
        gaze_options.put("device", "gazecontrollerclient");
        gaze_options.put("local", "/"+name+"/gaze");
        gaze_options.put("remote", "/iKinGazeCtrl");
        if (!gaze.open(gaze_options)) {
            yError() << "Unable to open gaze driver!";
            arm_r.close();
            arm_l.close();
            hand_r.close();
            hand_l.close();
            return false;
        }

        // set up velocity of arms' movements
        {
            vector<PolyDriver*> polys({&arm_r, &arm_l});
            for (auto poly:polys) {
                ICartesianControl* iarm;
                poly->view(iarm);
                iarm->setTrajTime(.6);
            }
        }

        // enable position control of the fingers
        {
            vector<PolyDriver*> polys({&hand_r, &hand_l});
            for (auto poly:polys) {
                IControlMode* imod;
                poly->view(imod);
                imod->setControlModes(fingers.size(), fingers.data(), vector<int>(fingers.size(), VOCAB_CM_POSITION).data());
            }
        }

        vector<string> objects_names{"mustard_bottle", "pudding_box"};
        for (const auto& object_name:objects_names) {
            objMoverPorts.push_back(make_pair(object_name, shared_ptr<BufferedPort<Bottle>>(new BufferedPort<Bottle>())));
            objMoverPorts.back().second->open("/"+name+"/"+object_name+"/mover:o");
        }

        rgbPort.open("/"+name+"/rgb:i");
        depthPort.open("/"+name+"/depth:i");
        sqPort.open("/"+name+"/sq:rpc");
        rpcPort.open("/"+name+"/rpc");
        attach(rpcPort);

        viewer = unique_ptr<Viewer>(new Viewer(10, 370, 350, 350));
        viewer->start();

        return true;
    }

    /**************************************************************************/
    bool go() override {
        if (randomize()) {
            if (home()) {
                if (segment()) {
                    if (fit()) {
                        if (grasp()) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    /**************************************************************************/
    bool randomize() override {
        random_device rnd_device;
        mt19937 mersenne_engine(rnd_device());
        uniform_real_distribution<double> dist_p(0., (double)(objMoverPorts.size() - 1));
        const auto i = (size_t)round(dist_p(mersenne_engine));

        auto port = objMoverPorts[i].second;
        if (port->getOutputCount() > 0) {
            uniform_real_distribution<double> dist_r(0., .05);
            uniform_real_distribution<double> dist_ang(90., 270.);
            uniform_real_distribution<double> dist_rot(-180., 180.);

            Bottle delta_pose;
            auto r = dist_r(mersenne_engine);
            auto ang = dist_ang(mersenne_engine) * (M_PI / 180.);
            delta_pose.addDouble(-.35 + r * cos(ang));
            delta_pose.addDouble(r * sin(ang));
            delta_pose.addDouble(dist_rot(mersenne_engine) * (M_PI / 180.));
            port->prepare() = delta_pose;
            port->writeStrict();

            return true;
        }
        
        return false;
    }

    /**************************************************************************/
    bool home() override {
        // home gazing
        IGazeControl* igaze;
        gaze.view(igaze);
        igaze->lookAtAbsAnglesSync({0., -50., 10.});

        // home arms
        {
            Vector x{-.25, .3, .1};
            vector<PolyDriver*> polys({&arm_r, &arm_l});
            ICartesianControl* iarm;
            for (auto poly:polys) {                
                poly->view(iarm);
                iarm->goToPositionSync(x);                
                x[1] = -x[1];
            }
            // wait only for the last arm
            iarm->waitMotionDone();
        }

        igaze->waitMotionDone();
        return true;
    }

    /**************************************************************************/
    bool segment() override {
        // get camera extrinsic matrix
        IGazeControl* igaze;
        gaze.view(igaze);
        Vector cam_x, cam_o;
        igaze->getLeftEyePose(cam_x, cam_o);
        Teye = axis2dcm(cam_o);
        Teye.setSubcol(cam_x, 0, 3);

        // get image data
        ImageOf<PixelRgb>* rgbImage = rgbPort.read();
        ImageOf<PixelFloat>* depthImage = depthPort.read();

        if ((rgbImage == nullptr) || (depthImage == nullptr)) {
            yError() << "Unable to receive image data!";
            return false;
        }

        if ((rgbImage->width() != depthImage->width()) ||
            (rgbImage->height() != depthImage->height()) ) {
            yError() << "Received image data with wrong size!";
            return false;
        }

        const auto w = rgbImage->width();
        const auto h = rgbImage->height();

        // aggregate image data in the point cloud of the whole scene
        pc_scene = shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>>(new yarp::sig::PointCloud<DataXYZRGBA>);
        const auto fov_h = (w / 2.) / tan((view_angle / 2.) * (M_PI / 180.));
        Vector x{0., 0., 0., 1.};
        for (int v = 0; v < h; v++) {
            for (int u = 0; u < w; u++) {
                const auto rgb = (*rgbImage)(u, v);
                const auto depth = (*depthImage)(u, v);
                
                if (depth > 0.F) {
                    x[0] = depth * (u - .5 * (w - 1)) / fov_h;
                    x[1] = depth * (v - .5 * (h - 1)) / fov_h;
                    x[2] = depth;
                    x = Teye * x;
                
                    pc_scene->push_back(DataXYZRGBA());
                    auto& p = (*pc_scene)(pc_scene->size() - 1);
                    p.x = (float)x[0];
                    p.y = (float)x[1];
                    p.z = (float)x[2];
                    p.r = rgb.r;
                    p.g = rgb.g;
                    p.b = rgb.b;
                    p.a = 255;
                }
            }
        }

        //savePCL("/workspace/pc_scene.off", pc_scene);

        // segment out the table and the object
        pc_table = shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>>(new yarp::sig::PointCloud<DataXYZRGBA>);
        pc_object = shared_ptr<yarp::sig::PointCloud<DataXYZRGBA>>(new yarp::sig::PointCloud<DataXYZRGBA>);
        table_height = Segmentation::RANSAC(pc_scene, pc_table, pc_object);
        if (isnan(table_height)) {
            yError() << "Segmentation failed!";
            return false;
        }

        //savePCL("/workspace/pc_table.off", pc_table);
        //savePCL("/workspace/pc_object.off", pc_object);

        // update viewer
        Vector cam_foc;
        igaze->get3DPointOnPlane(0, {w/2., h/2.}, {0., 0., 1., -table_height}, cam_foc);
        viewer->addTable({cam_foc[0], cam_foc[1], cam_foc[2]}, {0., 0., 1.});
        viewer->addObject(pc_object);
        viewer->addCamera({cam_x[0], cam_x[1], cam_x[2]}, {cam_foc[0], cam_foc[1], cam_foc[2]},
                          {0., 0., 1.}, view_angle);

        if (pc_object->size() > 0) {
            return true;
        } else {
            yError() << "Unable to segment any object!";
            return false;
        }
    }

    /**************************************************************************/
    bool fit() override {
        if (pc_object) {
            if (pc_object->size() > 0) {
                if (sqPort.getOutputCount() > 0) {
                    // offload object fitting to the external solver
                    const auto ret = sqPort.write(*pc_object, sqParams);
                    if (ret) {
                        viewer->addSuperquadric(sqParams);
                        return true;
                    } else {
                        yError() << "Unable to fit the object!";
                        return false;
                    }
                } else {
                    yError() << "Not connected to the solver!";
                    return false;
                }
            }
        }
        
        yError() << "No object to fit!";
        return false;
    }

    /**************************************************************************/
    bool grasp() override {
        if (sqParams.size() == 0) {
            yError() << "No object to grasp!";
            return false;
        }

        viewer->focusOnSuperquadric();

        const Vector sqCenter{sqParams.get(0).asDouble(),
                              sqParams.get(1).asDouble(),
                              sqParams.get(2).asDouble()};

        // keep gazing at the object
        IGazeControl* igaze;
        gaze.view(igaze);
        igaze->setTrackingMode(true);
        igaze->lookAtFixationPoint(sqCenter);

        // set up the hand pre-grasp configuration
        IControlLimits* ilim;
        hand_r.view(ilim);
        double pinkie_min, pinkie_max;
        ilim->getLimits(15, &pinkie_min, &pinkie_max);
        const vector<double> pregrasp_fingers_posture{60., 80., 0., 0., 0., 0., 0., 0., pinkie_max};

        // apply cardinal points grasp algorithm
        ICartesianControl* iarm;
        arm_r.view(iarm);
        auto grasper_r = make_shared<CardinalPointsGrasp>(CardinalPointsGrasp("right", pregrasp_fingers_posture));
        const auto candidates_r = grasper_r->getCandidates(sqParams, iarm);

        arm_l.view(iarm);
        auto grasper_l = make_shared<CardinalPointsGrasp>(CardinalPointsGrasp("left", pregrasp_fingers_posture));
        const auto candidates_l = grasper_l->getCandidates(sqParams, iarm);

        // aggregate right and left arms candidates
        auto candidates = candidates_r.first;
        candidates.insert(candidates.end(), candidates_l.first.begin(), candidates_l.first.end());
        std::sort(candidates.begin(), candidates.end(), CardinalPointsGrasp::compareCandidates);

        // some safety checks
        if (candidates.empty()) {
            yError() << "No good grasp candidates found!";
            lookAtDeveloper();
            shrug();
            return false;
        }

        // extract relevant info
        const auto& best = candidates[0];
        const auto& type = get<0>(best);
        const auto& T = get<2>(best);

        viewer->showCandidates(candidates);

        // select arm corresponing to the best candidate
        shared_ptr<CardinalPointsGrasp> grasper;
        IPositionControl* ihand;
        int context;
        if (type == "right") {
             grasper = grasper_r;
             hand_r.view(ihand);
             arm_r.view(iarm);
             context = candidates_r.second;
        } else {
             grasper = grasper_l;
             hand_l.view(ihand);
             arm_l.view(iarm);
             context = candidates_l.second;
        }

        // target pose that allows grasping the object
        const auto x = T.getCol(3).subVector(0, 2);
        const auto o = dcm2axis(T);

        // enable the context used by the algorithm
        iarm->stopControl();
        iarm->restoreContext(context);
        iarm->setInTargetTol(.001);
        iarm->setTrajTime(1.);

        // put the hand in the pre-grasp configuration
        ihand->setRefAccelerations(fingers.size(), fingers.data(), vector<double>(fingers.size(), numeric_limits<double>::infinity()).data());
        ihand->setRefSpeeds(fingers.size(), fingers.data(), vector<double>{60., 60., 60., 60., 60., 60., 60., 60., 200.}.data());
        ihand->positionMove(fingers.size(), fingers.data(), pregrasp_fingers_posture.data());
        auto done = false;
        while (!done) {
            Time::delay(1.);
            ihand->checkMotionDone(fingers.size(), fingers.data(), &done);
        };

        // reach for the pre-grasp pose
        const auto dir = x - sqCenter;
        iarm->goToPoseSync(x + .05 * dir / norm(dir), o);
        iarm->waitMotionDone(.1, 3.);
        
        // reach for the object
        iarm->goToPoseSync(x, o);
        iarm->waitMotionDone(.1, 3.);

        // close fingers
        ihand->positionMove(fingers.size(), fingers.data(), vector<double>{60., 80., 40., 35., 40., 35., 40., 35., pinkie_max}.data());

        // give enough time to adjust the contacts
        Time::delay(5.);

        // lift up the object
        const auto lift = x + Vector{0., 0., .1};
        igaze->lookAtFixationPoint(lift);
        iarm->goToPoseSync(lift, o);
        iarm->waitMotionDone(.1, 3.);

        lookAtDeveloper();
        return true;
    }

    /**************************************************************************/
    double getPeriod() override {
        return 1.0;
    }

    /**************************************************************************/
    bool updateModule() override {
        return true;
    }

    bool interruptModule() override {
        viewer->stop();

        // interrupt blocking read
        sqPort.interrupt();
        depthPort.interrupt();
        rgbPort.interrupt();
        return true;
    }

    /**************************************************************************/
    bool close() override {
        // restore default contexts
        IGazeControl* igaze;
        gaze.view(igaze);
        igaze->stopControl();
        igaze->restoreContext(0);

        vector<PolyDriver*> polys({&arm_r, &arm_l});
        for (auto poly:polys) {
            ICartesianControl* iarm;
            poly->view(iarm);
            iarm->stopControl();
            iarm->restoreContext(0);
        }

        rpcPort.close();
        sqPort.close();
        depthPort.close();
        rgbPort.close();
        for (auto port:objMoverPorts) {
            port.second->close();
        }
        gaze.close();
        arm_r.close();
        arm_l.close();
        hand_r.close();
        hand_l.close();
        return true;
    }
};

/******************************************************************************/
int main(int argc, char *argv[]) {
    Network yarp;
    if (!yarp.checkNetwork()) {
        yError() << "Unable to find YARP server!";
        return EXIT_FAILURE;
    }

    ResourceFinder rf;
    rf.configure(argc,argv);

    GrasperModule module;
    return module.runModule(rf);
}
