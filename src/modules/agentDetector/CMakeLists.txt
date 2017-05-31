# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Maxime Petit
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(agentDetector)

file(GLOB folder_source *.h *.cpp)
set(docs ${PROJECT_NAME}.xml)

source_group("Source Files" FILES ${folder_source})
source_group("DOC Files"    FILES ${docs})

include_directories(${kinectWrapper_INCLUDE_DIRS} ${icubhri_INCLUDE_DIRS} ${YARP_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${folder_source} ${docs})
target_link_libraries(${PROJECT_NAME} kinectWrapper icubhri ${YARP_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER humanUnderstanding)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${docs} DESTINATION ${ICUBHRI_MODULES_INSTALL_DIR})

