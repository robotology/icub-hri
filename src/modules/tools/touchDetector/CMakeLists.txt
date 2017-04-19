# Copyright: (C) 2015 iCub Facility - Istituto Italiano di Tecnologia
# Authors: Bertrand Higy
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(touchDetector)

file(GLOB folder_source src/*.cpp)
file(GLOB folder_header include/*.h)
set(doc_files ${PROJECT_NAME}.xml)

source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})
source_group("DOC Files"    FILES ${doc_files})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${icubclient_INCLUDE_DIRS} ${YARP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${folder_source} ${folder_header} ${doc_files})
target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER tools)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc_files} DESTINATION ${ICUBCLIENT_MODULES_INSTALL_DIR})
