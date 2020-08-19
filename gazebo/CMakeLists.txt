################################################################################
#                                                                              #
# Copyright (C) 2020 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# Install models
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION share/gazebo)

# Install worlds
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/worlds DESTINATION share/gazebo)
