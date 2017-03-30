# Copyright: (C) 2015 WYSIWYD Consortium
# Authors: Clément Moulin-Frier, Jordi Puigbo 
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname sensationManager)
file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/conf/*.ini)
yarp_install(FILES ${conf} DESTINATION ${ICUB_CLIENT_CONTEXTS_INSTALL_DIR}/${appname})

