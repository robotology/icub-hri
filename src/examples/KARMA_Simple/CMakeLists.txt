# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Nguyen Dong Hai Phuong
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(KARMA_Simple)

set(source_files main.cpp)
source_group("Source Files" FILES ${source_files})

include_directories(${icubclient_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${source_files})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples)
target_link_libraries(${PROJECT_NAME} icubclient ${YARP_LIBRARIES})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

