# Copyright: (C) 2016 WYSIWYD Consortium
# Authors: Ugo Pattacini
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname actionRecogDataDumper)

file(GLOB scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.*)
yarp_install(FILES ${scripts} DESTINATION ${ICUBHRI_APPLICATIONS_TEMPLATES_INSTALL_DIR})

file(GLOB ini ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.*)
yarp_install(FILES ${ini} ${grxml} DESTINATION ${ICUBHRI_CONTEXTS_INSTALL_DIR}/${appname})

