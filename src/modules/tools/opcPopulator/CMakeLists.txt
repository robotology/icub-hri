# Copyright: (C) 2015WYSIWYD Consortium
# Authors: Grégoire Pointeau
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

project(opcPopulator)

file(GLOB folder_source src/*.cpp src/*.cc src/*.c)
file(GLOB folder_header include/*.h  )
set(doc_files ${PROJECT_NAME}.xml)


source_group("Header Files" FILES ${folder_header})
source_group("Source Files" FILES ${folder_source})
source_group("DOC Files"    FILES ${doc_files})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${icubclient_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${folder_header} ${folder_source} ${doc_files})
target_link_libraries(${PROJECT_NAME} icubclient ${YARP_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER tools)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc_files} DESTINATION ${ICUBCLIENT_MODULES_INSTALL_DIR})

