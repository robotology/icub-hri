# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Maxime Petit
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

#
# demoModule CMake file, freely inspired by the iCub demoModule.
#

project(pasar)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
   
    ${icubclient_INCLUDE_DIRS}
)

# Search for source code.
file(GLOB sources src/*.cpp)
file(GLOB headers include/*.h)

set(docs ${PROJECT_NAME}.xml)

source_group("Source Files" FILES ${sources})
source_group("Header Files" FILES ${headers})
source_group("DOC Files"    FILES ${docs})

add_executable(${PROJECT_NAME} ${headers} ${sources} ${docs})
target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES} icubclient)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER attentionRelated)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${docs} DESTINATION ${ICUBCLIENT_MODULES_INSTALL_DIR})
