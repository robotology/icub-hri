# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Bertrand Higy
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname touchDetector)

file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini)
file(GLOB lua  ${CMAKE_CURRENT_SOURCE_DIR}/lua/*.lua)
file(GLOB template  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.xml.template)

yarp_install(FILES ${conf} DESTINATION ${ICUB_CLIENT_CONTEXTS_INSTALL_DIR}/${appname})
yarp_install(FILES ${lua}  DESTINATION ${ICUB_CLIENT_CONTEXTS_INSTALL_DIR}/${appname})
yarp_install(FILES ${template} DESTINATION ${ICUB_CLIENT_APPLICATIONS_TEMPLATES_INSTALL_DIR})

