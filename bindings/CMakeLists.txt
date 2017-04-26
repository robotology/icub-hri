# This is a CMake example for Python
if(WIN32)
    cmake_minimum_required(VERSION 3.4)
else()
    cmake_minimum_required(VERSION 3.0)
endif()

include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(SWIG REQUIRED 3)
find_package(icubclient REQUIRED)
find_package(YARP REQUIRED)
find_package(PythonLibs)

include(${SWIG_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PYTHON_INCLUDE_PATH} "/usr/local/src/robot/install/icub-client/include" ${YARP_INCLUDE_DIRS})

set(CMAKE_SWIG_FLAGS "-module;icubclient;-threads")

set_source_files_properties(icubclient.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(icubclient.i PROPERTIES SWIG_FLAGS "-c++")
swig_add_module(icubclient python icubclient.i)
target_link_libraries(_icubclient ${PYTHON_LIBRARIES} ${YARP_LIBRARIES} ${icubclient_LIBRARIES})
set_target_properties(_icubclient PROPERTIES OUTPUT_NAME "_icubclient")

