# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Stéphane Lallée
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(referenceFrameHandler)

set(folder_source main.cpp)
set(doc_files ${PROJECT_NAME}.xml)

source_group("Source Files" FILES ${folder_source})
source_group("DOC Files"    FILES ${doc_files})

include_directories(${YARP_INCLUDE_DIRS} ${ICUB_INCLUDE_DIRS} ${icubclient_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${folder_source} ${doc_files})
target_link_libraries(${PROJECT_NAME} optimization ${YARP_LIBRARIES} icubclient)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER tools)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc_files} DESTINATION ${ICUBCLIENT_MODULES_INSTALL_DIR})

