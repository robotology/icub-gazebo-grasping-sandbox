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
class ModelMover : public ModelPlugin
{
    gazebo::physics::ModelPtr model;
    gazebo::event::ConnectionPtr renderer_connection;
    yarp::os::BufferedPort<yarp::os::Bottle> port;

    /**************************************************************************/
    void onWorldFrame() {
        if (auto* v = port.read(false)) {
            if (v->size() >= 3) {
                const auto x = v->get(0).asDouble();
                const auto y = v->get(1).asDouble();
                const auto ang = v->get(2).asDouble() / 2.;
                const auto& p = model->WorldPose();
                const ignition::math::Quaterniond q(std::cos(ang), 0., 0., std::sin(ang));
                ignition::math::Pose3d pose(x, y, p.Pos()[2], q.W(), q.X(), q.Y(), q.Z());
                model->SetWorldPose(pose);
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
