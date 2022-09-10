#!/bin/bash

################################################################################
#                                                                              #
# Copyright (C) 2020 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# launch the demo
run() {
    yarpserver --write --silent &
    yarpmanager-console --application ${ROBOTOLOGY_SUPERBUILD_INSTALL_DIR}/share/ICUBcontrib/applications/icub-grasp.xml --run --connect --exit --silent
    
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
}

# clean up hanging up resources
clean() {
    declare -a modules=("icub-gazebo-grasping-sandbox" "find-superquadric" \
                        "yarpview" "iKinGazeCtrl" "iKinCartesianSolver" \
                        "yarprobotinterface" "gzclient" "gzserver" "yarpserver")
    for module in ${modules[@]}; do
        killall -9 ${module}
    done
}

# main
if [[ $# -eq 0 ]]; then
    echo "demo is starting up..."
    run
    echo "...demo done"
elif [ "$1" == "clean" ]; then
    echo "cleaning up resources..."
    clean
    echo "...cleanup done"
else
    echo "unknown option!"
fi
