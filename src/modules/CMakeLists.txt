# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Ugo Pattacini
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

#Add here modules without specific dependency
add_subdirectory(babbling)
add_subdirectory(ears)
add_subdirectory(pasar)
add_subdirectory(proactiveTagging)
add_subdirectory(planner)
add_subdirectory(reactiveLayer)
add_subdirectory(samRelated)
add_subdirectory(tools)

#Add modules with dependencies here
if(ICUBCLIENT_USE_OpenCV)
    add_subdirectory(faceTracker)
    add_subdirectory(iol2opc)
endif()

if(ICUBCLIENT_USE_kinectWrapper)
    add_subdirectory(agentDetector)
endif()

