# Copyright: (C) 2014 WYSIWYD Consortium
# Authors: Ugo Pattacini
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

if(MSVC)

   if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
   else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2")
   endif()
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251")

   add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
   set(CMAKE_DEBUG_POSTFIX "d")

elseif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
   # enable all warnings + extra warnings
   set(WARNINGS_LIST -Wall -Wextra)
   foreach(WARNING ${WARNINGS_LIST})
      set(WARNINGS_STRING "${WARNINGS_STRING} ${WARNING}")
   endforeach(WARNING)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS_STRING}")
endif()

if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
       set(CMAKE_BUILD_TYPE "Release" CACHE STRING
           "Choose the build type, recommended options are: Debug or Release" FORCE)
    endif()
    set(ICUBCLIENT_BUILD_TYPES "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${ICUBCLIENT_BUILD_TYPES})
endif()
