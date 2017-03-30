# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Martina Zambelli
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname babbling)
file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.*)
file(GLOB template ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.template)

yarp_install(FILES ${conf} DESTINATION ${ICUB_CLIENT_CONTEXTS_INSTALL_DIR}/${appname})
yarp_install(FILES ${template} DESTINATION ${ICUB_CLIENT_APPLICATIONS_TEMPLATES_INSTALL_DIR})
