# This is a CMake example for Python
if(WIN32)
    cmake_minimum_required(VERSION 3.4)
else()
    cmake_minimum_required(VERSION 3.0)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../conf)

include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include(icubclientOptions)

find_package(SWIG REQUIRED 3)
find_package(icubclient REQUIRED)
find_package(YARP REQUIRED)

get_target_property(icubclient_includes icubclient::icubclient INTERFACE_INCLUDE_DIRECTORIES)

set(ICUBCLIENT_USE_PYTHON_VERSION "" CACHE STRING "Specify python version to use" )
find_package(PythonInterp ${ICUBCLIENT_USE_PYTHON_VERSION} REQUIRED)
set(ICUBCLIENT_USE_PYTHON_VERSION_ARGS)
if(NOT ICUBCLIENT_USE_PYTHON_VERSION)
  set (ICUBCLIENT_USE_PYTHON_VERSION ${PYTHON_VERSION_STRING})
endif()

find_package(PythonLibs ${ICUBCLIENT_USE_PYTHON_VERSION} EXACT)

include(${SWIG_USE_FILE})

set(target_name icubclient)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PYTHON_INCLUDE_PATH} ${icubclient_includes} ${YARP_INCLUDE_DIRS})

set(CMAKE_SWIG_FLAGS "-module;${target_name};-threads")

set_source_files_properties(icubclient.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(icubclient.i PROPERTIES SWIG_FLAGS "-c++")
swig_add_module(${target_name} python icubclient.i)
target_link_libraries(_${target_name} ${PYTHON_LIBRARIES} ${YARP_LIBRARIES} ${icubclient_LIBRARIES})
set_target_properties(_${target_name} PROPERTIES OUTPUT_NAME "_icubclient")

# installation path is determined reliably on most platforms using distutils
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_python_lib(1,0,prefix=''))"
                OUTPUT_VARIABLE PYTHON_INSTDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/icubclient.py DESTINATION ${PYTHON_INSTDIR})

set(_CMAKE_INSTALL_PYTHONDIR "${PYTHON_INSTDIR}")
set(CMAKE_INSTALL_PYTHONDIR ${_CMAKE_INSTALL_PYTHONDIR} CACHE PATH "python bindings (${_CMAKE_INSTALL_PYTHONDIR})")
mark_as_advanced(CMAKE_INSTALL_PYTHONDIR)
if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_PYTHONDIR})
  set(CMAKE_INSTALL_FULL_PYTHONDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_PYTHONDIR}")
else()
  set(CMAKE_INSTALL_FULL_PYTHONDIR "${CMAKE_INSTALL_PYTHONDIR}")
endif()

# Update RPATH
if(NOT CMAKE_SKIP_RPATH AND NOT CMAKE_SKIP_INSTALL_RPATH)
  file(RELATIVE_PATH _rel_path "${CMAKE_INSTALL_FULL_PYTHONDIR}" "${CMAKE_INSTALL_FULL_LIBDIR}")
  get_target_property(_current_rpath _${target_name} INSTALL_RPATH)
  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list(APPEND _current_rpath "@loader_path/${_rel_path}")
  else()
    list(APPEND _current_rpath "\$ORIGIN/${_rel_path}")
  endif()
  set_target_properties("_${target_name}" PROPERTIES INSTALL_RPATH "${_current_rpath}")
endif()

install(TARGETS _${target_name} DESTINATION ${PYTHON_INSTDIR})

