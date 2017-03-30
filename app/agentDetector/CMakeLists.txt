# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Maxime Petit
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname agentDetector)
file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.*)
yarp_install(FILES ${conf} DESTINATION ${ICUB_CLIENT_CONTEXTS_INSTALL_DIR}/${appname})
file(GLOB template ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.*)
yarp_install(FILES ${template} DESTINATION ${ICUB_CLIENT_APPLICATIONS_TEMPLATES_INSTALL_DIR})

