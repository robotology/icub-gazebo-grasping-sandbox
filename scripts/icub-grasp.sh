#!/bin/bash

yarpserver --write --silent &
yarpdev --device transformServer --ROS::enable_ros_publisher false --ROS::enable_ros_subscriber false
gazebo -e dart icub-gazebo-grasping-sandbox.sdf &

sleep 5
yarp wait /icubSim/torso/state:o
yarp wait /icubSim/head/state:o
yarp wait /icubSim/left_arm/state:o
yarp wait /icubSim/right_arm/state:o

yarpmanager-console --application ${ICUBcontrib_DIR}/share/ICUBcontrib/applications/icub-grasp.xml --run --connect --exit --silent

sleep 10
declare -a modules=("yarpview" "iKinGazeCtrl" "iKinCartesianSolver" "yarprobotinterface" "gzclient" "gzserver" "yarpdev" "yarpserver")
for module in ${modules[@]}; do
  killall ${module}
done
