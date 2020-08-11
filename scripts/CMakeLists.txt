################################################################################
#                                                                              #
# Copyright (C) 2020 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# Install the application
yarp_install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/icub-grasp.xml DESTINATION ${ICUBCONTRIB_APPLICATIONS_INSTALL_DIR})

# Install startup script
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/icub-grasp.sh ${CMAKE_CURRENT_SOURCE_DIR}/gc-icub-grasp.sh DESTINATION bin)
