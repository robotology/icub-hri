# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Ugo Pattacini
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(LIBRARY_TARGET_NAME icubclientIDLClients)

file(GLOB ${LIBRARY_TARGET_NAME}_HDR ${IDL_GENERATED_CODE_DIR}/include/*.h)
file(GLOB ${LIBRARY_TARGET_NAME}_SRC ${IDL_GENERATED_CODE_DIR}/src/*.cpp)

list(APPEND ${LIBRARY_TARGET_NAME}_HDR ${LIBRARY_TARGET_NAME}.h)

source_group("Header Files" FILES ${LIBRARY_TARGET_NAME}_HDR)
source_group("Source Files" FILES ${LIBRARY_TARGET_NAME}_SRC)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRC} ${${LIBRARY_TARGET_NAME}_HDR})
target_link_libraries(${LIBRARY_TARGET_NAME} ${YARP_LIBRARIES})

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES DEBUG_POSTFIX "d"
                                                        VERSION       ${${PROJECT_NAME}_VERSION}
                                                        LINKER_LANGUAGE CXX)

target_include_directories(${LIBRARY_TARGET_NAME} SYSTEM PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${YARP_INCLUDE_DIRS};${IDL_GENERATED_CODE_DIR}/include>"
                                                                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>")

# Specify installation targets, typology and destination folders.
install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)

set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${LIBRARY_TARGET_NAME})

