#!/bin/bash

yarpserver --write --silent &
gazebo -e dart icub-gazebo-grasping-sandbox.sdf &

sleep 5
yarp wait /icubSim/torso/state:o
yarp wait /icubSim/head/state:o
yarp wait /icubSim/left_arm/state:o
yarp wait /icubSim/right_arm/state:o

yarpmanager-console --application icub-grasp.xml --run --connect --exit --silent
sleep 10
yarpmanager-console --application icub-grasp.xml --kill --exit --silent

declare -a modules=("gzclient" "gzserver" "yarpserver")
for module in ${modules[@]}; do
  killall ${module}
done