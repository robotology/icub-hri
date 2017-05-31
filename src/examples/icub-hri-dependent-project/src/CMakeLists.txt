# Copyright: (C) 2017 WYSIWYD Consortium
# Authors: Tobias Fischer
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(exampledependency)
include_directories(${YARP_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER tools)
target_link_libraries(${PROJECT_NAME} ${icubhri_LIBRARIES} ${YARP_LIBRARIES})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
