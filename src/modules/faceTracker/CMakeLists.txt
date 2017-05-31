# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Hyung Jin Chang
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

if(${OpenCV_VERSION_MAJOR} LESS 3)
    message(WARNING "OpenCV 3 is required for faceTracker to compile.")
else()
    project(faceTracker)

    include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${YARP_INCLUDE_DIRS}
        ${icubhri_INCLUDE_DIRS}
    )

    # Search for source code.
    file(GLOB sources src/*.cpp)
    file(GLOB headers include/*.h)

    set(docs ${PROJECT_NAME}.xml)

    source_group("Source Files" FILES ${sources})
    source_group("Header Files" FILES ${headers})
    source_group("DOC Files"    FILES ${docs})


    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                        ${icubhri_INCLUDE_DIRS}
                        ${YARP_INCLUDE_DIRS}
                        ${OpenCV_INCLUDE_DIRS})

    add_executable(${PROJECT_NAME} ${headers} ${sources} ${docs})
    target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES} icubhri ${OpenCV_LIBRARIES})
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER faceTracker)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    yarp_install(FILES ${docs} DESTINATION ${ICUBHRI_MODULES_INSTALL_DIR})
endif()
