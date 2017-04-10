# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Maxime Petit
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(agentDetector)

file(GLOB folder_source *.h *.cpp)
source_group("Source Files" FILES ${folder_source})

include_directories(${kinectWrapper_INCLUDE_DIRS} ${icubclient_INCLUDE_DIRS} ${YARP_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${folder_source})
target_link_libraries(${PROJECT_NAME} kinectWrapper icubclient ${YARP_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER humanUnderstanding)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${docs} DESTINATION ${ICUBCLIENT_MODULES_INSTALL_DIR})

