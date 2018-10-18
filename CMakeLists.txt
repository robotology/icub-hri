# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Ugo Pattacini, Tobias Fischer
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.5)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(icubclient
        LANGUAGES CXX
        VERSION 3.1.0.0)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ICUBCLIENT_COMPILE_BINDINGS "Compile optional language bindings" FALSE)

find_package(YARP 3.0.1 REQUIRED)
find_package(ICUB)

# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros
# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE
# See https://cmake.org/cmake/help/v3.4/variable/CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS.html
# See https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/conf)

message(STATUS "YARP is version: ${YARP_VERSION}")
message(STATUS "iCub is version: ${ICUB_VERSION}")

yarp_configure_external_installation(${PROJECT_NAME})

### Options
# Shared/Dynamic or Static library?
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)

# Support RPATH?
option(ENABLE_RPATH "Enable RPATH for this library" ON)
mark_as_advanced(ENABLE_RPATH)
include(AddInstallRPATHSupport)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
                          LIB_DIRS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
                          DEPENDS ENABLE_RPATH
                          USE_LINK_PATH)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
message(STATUS "Libraries go to ${LIBRARY_OUTPUT_PATH}")
message(STATUS "Executables go to ${EXECUTABLE_OUTPUT_PATH}")
make_directory(${LIBRARY_OUTPUT_PATH})
make_directory(${EXECUTABLE_OUTPUT_PATH})
mark_as_advanced(LIBRARY_OUTPUT_PATH)
mark_as_advanced(EXECUTABLE_OUTPUT_PATH)


set(IDL_GENERATED_CODE_DIR ${PROJECT_SOURCE_DIR}/src/idl_generated_code)
mark_as_advanced(IDL_GENERATED_CODE_DIR)
message(STATUS "IDL generated files go to ${IDL_GENERATED_CODE_DIR}")

include(icubclientFindDependencies)
include(icubclientOptions)

add_subdirectory(src)
add_subdirectory(app)

# Install the files necessary to call find_package(LibTemplateCMake) in CMake projects
include(InstallBasicPackageFiles)
install_basic_package_files(${PROJECT_NAME} VARS_PREFIX ${PROJECT_NAME}
                                            VERSION ${${PROJECT_NAME}_VERSION}
                                            COMPATIBILITY AnyNewerVersion
                                            TARGETS_PROPERTY ${PROJECT_NAME}_TARGETS
                                            NO_CHECK_REQUIRED_COMPONENTS_MACRO)

include(AddUninstallTarget)

#### install cmake scripts
message(STATUS "Installing cmake scripts")
set(ICUBCLIENT_CMAKE_SCRIPTS icubclientOptions.cmake)

foreach(conf_i ${ICUBCLIENT_CMAKE_SCRIPTS})
    configure_file(${CMAKE_SOURCE_DIR}/conf/${conf_i}
                   ${CMAKE_BINARY_DIR}/conf/${conf_i} COPYONLY IMMEDIATE)
    install(FILES conf/${conf_i} DESTINATION share/${PROJECT_NAME}/conf/ COMPONENT Development)
endforeach()


if(ICUBCLIENT_COMPILE_BINDINGS)
  add_subdirectory(bindings)
endif()

####

