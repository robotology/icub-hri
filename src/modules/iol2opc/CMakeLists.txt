# Copyright: (C) 2015 WYSIWYD Consortium
# Authors: Ugo Pattacini, Tobias Fischer
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(ICUBCLIENT_BUILD_IOL2OPC TRUE CACHE BOOL "Enable compilation of iol2opc")
if(ICUBCLIENT_BUILD_IOL2OPC)
   if(ICUB_FOUND AND (NOT (OpenCV_VERSION LESS 3)))
        project(iol2opc)

        set(folder_header include/utils.h
                         include/module.h
                         ${IDL_GENERATED_CODE_DIR}/include/${PROJECT_NAME}_IDL.h)

        set(folder_source src/utils.cpp
                         src/module.cpp
                         src/main.cpp
                         ${IDL_GENERATED_CODE_DIR}/src/${PROJECT_NAME}_IDL.cpp)

        set(docs ${PROJECT_NAME}.xml)

        set(idl_files ${PROJECT_NAME}.thrift)

        yarp_idl_to_dir(${idl_files} ${IDL_GENERATED_CODE_DIR})

        source_group("Header Files" FILES ${folder_header})
        source_group("Source Files" FILES ${folder_source})
        source_group("IDL Files"    FILES ${idl_files})
        source_group("DOC Files"    FILES ${docs})

        include_directories(${PROJECT_SOURCE_DIR}/include
                           ${IDL_GENERATED_CODE_DIR}/include
                           ${icubclient_INCLUDE_DIRS}
                           ${OpenCV_INCLUDE_DIRS}
                          
                           ${ICUB_INCLUDE_DIRS} )

        add_executable(${PROJECT_NAME} ${idl_files} ${folder_header} ${folder_source} ${docs})
        target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBRARIES} ctrlLib ${YARP_LIBRARIES} icubclient)

        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER systemVisual)
        install(TARGETS ${PROJECT_NAME} DESTINATION bin)
  else()
    message(WARNING "ICUBCLIENT_BUILD_IOL2OPC is ON but ICUB is not found or OpenCV3 is not installed. Will not compile iol2opc.")
  endif()
endif()

