################################################################################
#                                                                              #
# Copyright (C) 2020 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

find_package(YARP REQUIRED COMPONENTS os dev sig math)
find_package(ICUB REQUIRED COMPONENTS iKin)
find_package(VTK REQUIRED)
message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  include(${VTK_USE_FILE})
endif()
find_package(Gazebo REQUIRED)

yarp_add_idl(IDL_GEN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/rpc.thrift)
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${IDL_GEN_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
target_link_libraries(${PROJECT_NAME} PRIVATE ${YARP_LIBRARIES} ICUB::iKin ${VTK_LIBRARIES})

if (VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(TARGETS ${PROJECT_NAME} MODULES ${VTK_LIBRARIES})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

set(PLUGIN_TARGET_NAME ${PROJECT_NAME}-model-mover)
add_library(${PLUGIN_TARGET_NAME} SHARED model_mover.cpp)
target_include_directories(${PLUGIN_TARGET_NAME} PRIVATE ${GAZEBO_INCLUDE_DIRS})
target_link_libraries(${PLUGIN_TARGET_NAME} PRIVATE ${GAZEBO_LIBRARIES} YARP::YARP_os)
install(TARGETS ${PLUGIN_TARGET_NAME} LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib)
