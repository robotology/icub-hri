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
set(icubclient_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

foreach(lang "Java"
             "Python")
  string(TOUPPER "${lang}" LANG)
  string(REGEX REPLACE " .+" "" LANG "${LANG}")
  option(CREATE_${LANG} "Do you want to create the ${lang} interface" OFF)
  if(CREATE_${LANG})
    message(STATUS " +++ bindings ${lang} is enabled")
  else()
    message(STATUS " --- bindings ${lang} is not enabled")
  endif()
endforeach()

include(${SWIG_USE_FILE})
message(STATUS "${SWIG_USE_FILE}")

include_directories(SYSTEM ${YARP_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${icubclient_includes})

set_source_files_properties(icubclient.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(icubclient.i PROPERTIES SWIG_FLAGS "-c++")

if(CREATE_PYTHON)
    set(ICUBCLIENT_USE_PYTHON_VERSION "" CACHE STRING "Specify python version to use" )
    find_package(PythonInterp ${ICUBCLIENT_USE_PYTHON_VERSION} REQUIRED)
    set(ICUBCLIENT_USE_PYTHON_VERSION_ARGS)
    if(NOT ICUBCLIENT_USE_PYTHON_VERSION)
      set (ICUBCLIENT_USE_PYTHON_VERSION ${PYTHON_VERSION_STRING})
    endif()

    find_package(PythonLibs ${ICUBCLIENT_USE_PYTHON_VERSION} EXACT)

    set(target_name icubclient)

    include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})

    set(CMAKE_SWIG_FLAGS "-module;${target_name};-threads")

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
endif(CREATE_PYTHON)

if(CREATE_JAVA)
  find_package(JNI REQUIRED)
  find_program(JAVA_JAVAC_EXECUTABLE NAMES javac)

  option(PREPARE_CLASS_FILES "Prepare class files" OFF)
  set(JAVA_TARGET_VERSION 1.6 CACHE STRING "Java target version")
  set(CMAKE_SWIG_OUTDIR_SAVE ${CMAKE_SWIG_OUTDIR})
  set(CMAKE_SWIG_OUTDIR "${icubclient_BASE_DIR}/generated_src/java")
  set(CMAKE_SWIG_FLAGS "-package;icubclient;-Wall;-module;icubclient;${SWIG_COMMON_FLAGS}")
  set(TARGET_DIR generated_src/java)
  file(MAKE_DIRECTORY "${icubclient_BASE_DIR}/${TARGET_DIR}")
  if(NOT BINDINGS_SRC_DIR)
    set(BINDINGS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
  endif()
  set(JAVA_SRCS Loadicubclient.java)
  include_directories(SYSTEM ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
  set(target_name jicubclient)
  swig_add_module(${target_name} java icubclient.i)
  target_link_libraries(${target_name} ${JAVA_LIBRARIES} ${YARP_LIBRARIES} ${icubclient_LIBRARIES})
  if(PREPARE_CLASS_FILES)
    set(CLASS_FILES)
    foreach(java_file ${JAVA_SRCS})
      string(REPLACE ".java" ".class" class_file ${java_file})
      list(APPEND JAVA_FILES "${BINDINGS_SRC_DIR}/${java_file}")
      list(APPEND CLASS_FILES "${icubclient_BASE_DIR}/${class_file}")
    endforeach()
    add_custom_command(OUTPUT ${CLASS_FILES}
                       COMMAND ${JAVA_JAVAC_EXECUTABLE} -source ${JAVA_TARGET_VERSION}
                                                        -target ${JAVA_TARGET_VERSION}
                                                        -d "${icubclient_BASE_DIR}"
                                                        "${icubclient_BASE_DIR}/${TARGET_DIR}/*.java"
                       COMMAND ${JAVA_JAVAC_EXECUTABLE} -source ${JAVA_TARGET_VERSION}
                                                        -target ${JAVA_TARGET_VERSION}
                                                        -d "${icubclient_BASE_DIR}/"
                                                        "${BINDINGS_SRC_DIR}/*.java"
                       WORKING_DIRECTORY ${icubclient_BASE_DIR}
                       DEPENDS ${JAVA_FILES} ${target_name})
    add_custom_target(${target_name}_classes ALL DEPENDS ${CLASS_FILES})
  endif()

  # issue on MINGW where C++ name-mangling prevents java finding methods
  if(MINGW)
    message(STATUS "untested MINGW patch - see CMakeLists.txt")
    set_target_properties(${target_name} PROPERTIES
      LINK_FLAGS "--add-stdcall-alias")
  endif(MINGW)
  ## the following is based on:
  ## http://www.cmake.org/pipermail/cmake/2007-September/016343.html
  if(APPLE)
    set_target_properties(${target_name} PROPERTIES SUFFIX ".jnilib")
  endif(APPLE)
  set(CMAKE_SWIG_OUTDIR ${CMAKE_SWIG_OUTDIR_SAVE})
endif(CREATE_JAVA)
