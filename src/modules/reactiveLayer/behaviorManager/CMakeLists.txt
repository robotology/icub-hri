# Copyright: (C) 2015 WYSIWYD Consortium
# Authors: Clement Moulin-Frier
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(behaviorManager)

file(GLOB sources src/*.cpp)
file(GLOB headers include/*.h)
set(docs ${PROJECT_NAME}.xml)

source_group("Source Files" FILES ${sources})
source_group("Header Files" FILES ${headers})
source_group("DOC Files"    FILES ${docs})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${icubclient_INCLUDE_DIRS} ${YARP_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${headers} ${sources} ${docs})
target_link_libraries(${PROJECT_NAME} icubclient ${YARP_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER reactiveLayer)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc_files} DESTINATION ${ICUBCLIENT_MODULES_INSTALL_DIR})
