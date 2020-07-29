#!/bin/bash

yarpserver --write --silent &
gazebo -e dart icub-gazebo-grasping-sandbox.sdf &

sleep 5
yarp wait /icubSim/torso/state:o
yarp wait /icubSim/head/state:o
yarp wait /icubSim/left_arm/state:o
yarp wait /icubSim/right_arm/state:o

yarpmanager-console --application icub-grasp.xml --run --connect --exit --silent

sleep 5
declare -a modules=("yarpview" "iKinGazeCtrl" "iKinCartesianSolver" "yarprobotinterface" "gzclient" "gzserver" "yarpserver")
for module in ${modules[@]}; do
  killall ${module}
  sleep 3
done