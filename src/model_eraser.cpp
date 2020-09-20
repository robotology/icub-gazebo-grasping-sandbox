/******************************************************************************
 *                                                                            *
 * Copyright (C) 2020 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

#include <cmath>

#include <gazebo/common/Plugin.hh>
#include <gazebo/physics/World.hh>
#include <gazebo/common/Events.hh>

#include <boost/bind.hpp>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>

namespace gazebo {

/******************************************************************************/
class ModelEraser : public gazebo::WorldPlugin
{
    gazebo::physics::WorldPtr world;
    gazebo::event::ConnectionPtr renderer_connection;
    yarp::os::BufferedPort<yarp::os::Bottle> port;

    /**************************************************************************/
    void onWorld() {
        if (auto* b = port.read(false)) {
            if (b->size() > 0) { 
                world->RemoveModel(b->get(0).asString());
            }
        }
    }

public:
    /**************************************************************************/
    void Load(gazebo::physics::WorldPtr world, sdf::ElementPtr) {
        this->world = world;
        port.open("/world/model-eraser/model-name:i");
        auto bind = boost::bind(&ModelEraser::onWorld, this);
        renderer_connection = gazebo::event::Events::ConnectWorldUpdateBegin(bind);
    }

    /**************************************************************************/
    virtual ~ModelEraser() {
        if (!port.isClosed()) {
            port.close();
        }
    }
};

}

GZ_REGISTER_WORLD_PLUGIN(gazebo::ModelEraser)
