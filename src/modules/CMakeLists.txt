# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Ugo Pattacini
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

#Add here modules without specific dependency
add_subdirectory(attentionRelated)
add_subdirectory(babbling)
add_subdirectory(planner)
add_subdirectory(reactiveLayer)
add_subdirectory(samRelated)
add_subdirectory(tools)
add_subdirectory(touchDetector)

#Add modules with dependencies here
if(WYSIWYD_USE_OpenCV)
    add_subdirectory(faceTracker)
    add_subdirectory(iol2opc)
endif()

if(WYSIWYD_USE_kinectWrapper)
    add_subdirectory(agentDetector)
endif()

