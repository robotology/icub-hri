# Copyright: (C) 2017 WYSIWYD Consortium
# Authors: Tobias Fischer
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

if(WIN32)
    cmake_minimum_required(VERSION 3.4)
else()
    cmake_minimum_required(VERSION 3.0)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(icubclientdependency
        LANGUAGES CXX
        VERSION 0.0.0.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(YARP REQUIRED)
find_package(icubclient REQUIRED)
message(STATUS "YARP is version: ${YARP_VERSION}")
message(STATUS "icubclient is version: ${icubclient_VERSION}")
add_subdirectory(src)
