#!/bin/bash

yarpserver --write --silent &
gazebo -e dart icub-gazebo-grasping-sandbox.sdf &

sleep 20
yarp wait /icubSim/torso/state:o
yarp wait /icubSim/head/state:o
yarp wait /icubSim/left_arm/state:o
yarp wait /icubSim/right_arm/state:o

yarpmanager-console --application ${ICUBcontrib_DIR}/share/ICUBcontrib/applications/icub-grasp.xml --run --connect --exit --silent
yarp wait /icub-grasp/rpc

echo "go" | yarp rpc /icub-grasp/rpc

sleep 5
declare -a modules=("icub-gazebo-grasping-sandbox" "find-superquadric" "yarpview")
for module in ${modules[@]}; do
  killall ${module}
done

sleep 5
declare -a modules=("iKinGazeCtrl" "iKinCartesianSolver" "yarprobotinterface")
for module in ${modules[@]}; do
  killall ${module}
done

sleep 5
declare -a modules=("gzclient" "gzserver" "yarpserver")
for module in ${modules[@]}; do
  killall ${module}
done
