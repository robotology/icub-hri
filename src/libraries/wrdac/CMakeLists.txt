# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Stephane Lallee
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(LIBRARY_TARGET_NAME wrdac)

set(${LIBRARY_TARGET_NAME}_SRC src/functions.cpp
                  src/knowledge/action.cpp
                  src/knowledge/agent.cpp
                  src/knowledge/bodypart.cpp
                  src/knowledge/entity.cpp
                  src/knowledge/object.cpp
                  src/knowledge/relation.cpp
                  src/subsystems/subSystem_agentDetector.cpp
                  src/subsystems/subSystem_ARE.cpp
                  src/subsystems/subSystem_babbling.cpp
                  src/subsystems/subSystem_iol2opc.cpp
                  src/subsystems/subSystem_recog.cpp
                  src/subsystems/subSystem_speech.cpp
                  src/subsystems/subSystem_KARMA.cpp
                  src/clients/icubClient.cpp
                  src/clients/opcClient.cpp)


set(${LIBRARY_TARGET_NAME}_HDR include/wrdac/functions.h
                  include/wrdac/tags.h
                  include/wrdac/knowledge/action.h
                  include/wrdac/knowledge/agent.h
                  include/wrdac/knowledge/bodypart.h
                  include/wrdac/knowledge/entity.h
                  include/wrdac/knowledge/object.h
                  include/wrdac/knowledge/relation.h
                  include/wrdac/knowledge/representations.h
                  include/wrdac/clients/icubClient.h
                  include/wrdac/clients/opcClient.h
                  include/wrdac/subsystems/subSystem.h
                  include/wrdac/subsystems/subSystem_agentDetector.h
                  include/wrdac/subsystems/subSystem_ARE.h
                  include/wrdac/subsystems/subSystem_babbling.h
                  include/wrdac/subsystems/subSystem_iol2opc.h
                  include/wrdac/subsystems/subSystem_recog.h
                  include/wrdac/subsystems/subSystem_speech.h
                  include/wrdac/subsystems/subSystem_KARMA.h)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRC} ${${LIBRARY_TARGET_NAME}_HDR})
target_link_libraries(${LIBRARY_TARGET_NAME} ${YARP_LIBRARIES} icubClientIDLClients)

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES VERSION       ${${PROJECT_NAME}_VERSION}
                                                        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_HDR}"
                                                        DEBUG_POSTFIX "d")

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${icubClientIDLClients_INCLUDE_DIRS};${YARP_INCLUDE_DIRS}>"
                                                         "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>")

if(MSVC)
  target_compile_definitions(${LIBRARY_TARGET_NAME} PUBLIC _USE_MATH_DEFINES)
endif()

# Specify installation targets, typology and destination folders.
install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)

set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${LIBRARY_TARGET_NAME})

