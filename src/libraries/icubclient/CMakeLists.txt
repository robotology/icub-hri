# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Stephane Lallee
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(LIBRARY_TARGET_NAME icubclient)

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
                  src/subsystems/subSystem_SAM.cpp
                  src/subsystems/subSystem_emotion.cpp
                  src/clients/icubClient.cpp
                  src/clients/icubClient_ARErelated.cpp
                  src/clients/icubClient_KARMArelated.cpp
                  src/clients/opcClient.cpp)


set(${LIBRARY_TARGET_NAME}_HDR include/${LIBRARY_TARGET_NAME}/all.h
                  include/${LIBRARY_TARGET_NAME}/functions.h
                  include/${LIBRARY_TARGET_NAME}/tags.h
                  include/${LIBRARY_TARGET_NAME}/knowledge/action.h
                  include/${LIBRARY_TARGET_NAME}/knowledge/agent.h
                  include/${LIBRARY_TARGET_NAME}/knowledge/bodypart.h
                  include/${LIBRARY_TARGET_NAME}/knowledge/entity.h
                  include/${LIBRARY_TARGET_NAME}/knowledge/object.h
                  include/${LIBRARY_TARGET_NAME}/knowledge/relation.h
                  include/${LIBRARY_TARGET_NAME}/knowledge/representations.h
                  include/${LIBRARY_TARGET_NAME}/clients/icubClient.h
                  include/${LIBRARY_TARGET_NAME}/clients/opcClient.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_agentDetector.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_ARE.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_babbling.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_iol2opc.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_recog.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_speech.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_KARMA.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_SAM.h
                  include/${LIBRARY_TARGET_NAME}/subsystems/subSystem_emotion.h)

add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRC} ${${LIBRARY_TARGET_NAME}_HDR})
target_link_libraries(${LIBRARY_TARGET_NAME} ${YARP_LIBRARIES} icubclientIDLClients)

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES VERSION       ${${PROJECT_NAME}_VERSION}
                                                        DEBUG_POSTFIX "d"
                                                        LINKER_LANGUAGE CXX)

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${icubclientIDLClients_INCLUDE_DIRS};${YARP_INCLUDE_DIRS}>"
                                                         "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>")

if(MSVC)
  target_compile_definitions(${LIBRARY_TARGET_NAME} PUBLIC _USE_MATH_DEFINES)
endif()

# Specify installation targets, typology and destination folders.
install(TARGETS ${LIBRARY_TARGET_NAME}
        EXPORT  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin)
install(DIRECTORY include/${LIBRARY_TARGET_NAME}/ DESTINATION include/${LIBRARY_TARGET_NAME} COMPONENT dev)

set_property(GLOBAL APPEND PROPERTY ${PROJECT_NAME}_TARGETS ${LIBRARY_TARGET_NAME})

