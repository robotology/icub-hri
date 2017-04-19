# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Stéphane Lallée
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(appname demos)
file(GLOB templates ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.xml)
yarp_install(FILES ${templates} DESTINATION ${ICUBCLIENT_APPLICATIONS_TEMPLATES_INSTALL_DIR})

