#!/bin/bash

declare -a modules=("icub-gazebo-grasping-sandbox" "find-superquadric" \
                    "yarpview" "iKinGazeCtrl" "iKinCartesianSolver" \
                    "yarprobotinterface" "gzclient" "gzserver" \
                    "yarpdev" "yarpserver")
for module in ${modules[@]}; do
  killall -9 ${module}
done
