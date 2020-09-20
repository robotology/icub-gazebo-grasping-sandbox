/******************************************************************************
 *                                                                            *
 * Copyright (C) 2020 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

#include <cmath>

#include <gazebo/common/Plugin.hh>
#include <gazebo/physics/Model.hh>
#include <gazebo/common/Events.hh>
#include <ignition/math/Pose3.hh>

#include <boost/bind.hpp>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>

namespace gazebo {

/******************************************************************************/
class ModelMover : public gazebo::ModelPlugin
{
    gazebo::physics::ModelPtr model;
    gazebo::event::ConnectionPtr renderer_connection;
    yarp::os::BufferedPort<yarp::os::Bottle> port;

    /**************************************************************************/
    void onWorldFrame() {
        if (auto* b = port.read(false)) {
            if (b->size() >= 3) { 
                const auto x = b->get(0).asDouble();
                const auto y = b->get(1).asDouble();
                const auto ang = b->get(2).asDouble() / 2.;
                const auto& cur_pose = model->WorldPose();
                const auto q = cur_pose.Rot() * ignition::math::Quaterniond(std::cos(ang), 0., 0., std::sin(ang));
                ignition::math::Pose3d new_pose(x, y, cur_pose.Pos()[2], q.W(), q.X(), q.Y(), q.Z());
                model->SetWorldPose(new_pose);
            }
        }
    }

public:
    /**************************************************************************/
    void Load(gazebo::physics::ModelPtr model, sdf::ElementPtr) {
        this->model = model;
        port.open("/" + model->GetName() + "/model-mover/delta-pose:i");
        auto bind = boost::bind(&ModelMover::onWorldFrame, this);
        renderer_connection = gazebo::event::Events::ConnectWorldUpdateBegin(bind);
    }

    /**************************************************************************/
    virtual ~ModelMover() {
        if (!port.isClosed()) {
            port.close();
        }
    }
};

}

GZ_REGISTER_MODEL_PLUGIN(gazebo::ModelMover)
