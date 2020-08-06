################################################################################
#                                                                              #
# Copyright (C) 2020 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

find_package(YARP REQUIRED COMPONENTS os dev sig math)
yarp_add_idl(IDL_GEN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/rpc.thrift)
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${IDL_GEN_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
target_link_libraries(${PROJECT_NAME} PRIVATE ${YARP_LIBRARIES})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)