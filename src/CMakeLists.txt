# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Ugo Pattacini
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

add_subdirectory(libraries)
add_subdirectory(examples)
add_subdirectory(modules)

option(ICUBCLIENT_COMPILE_TESTS "Compile Tests" OFF)
if(ICUBCLIENT_COMPILE_TESTS)
  add_subdirectory(tests)
endif()

