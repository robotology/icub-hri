# Copyright: (C) 2016 WYSIWYD Consortium
# Authors: Ugo Pattacini
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(actionRecogDataDumper)
set(doc_files ${PROJECT_NAME}.xml)

source_group("DOC Files"    FILES ${doc_files})
source_group("Source Files" FILES ${folder_source})

include_directories(${icubclient_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER tools)
target_link_libraries(${PROJECT_NAME} icubclient ${YARP_LIBRARIES})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc_files} DESTINATION ${ICUBCLIENT_MODULES_INSTALL_DIR})
