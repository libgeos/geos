#################################################################################
#
# CMake configuration of GEOS tests based on Catch
#
# Copyright (C) 2017 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

set(STATUS_MESSAGE "Enable GEOS Catch-based Unit Tests build")
set(STATUS_RESULT "OFF")

if(GEOS_ENABLE_TESTS_UNIT2_ONLY)
  # TODO: Enable as soon as std::auto_pt ris gone
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX14)
    message(STATUS "GEOS test build: C++ Standard: C++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  elseif(COMPILER_SUPPORTS_CXX11)
    message(STATUS "GEOS test build: C++ Standard: C++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    message(STATUS "GEOS test build: C++ Standard: C++0x")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(WARNING "Compiler ${CMAKE_CXX_COMPILER} has no C++ 0x/11/14 support.")
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
    # Workaround for Travis CI + Catch: `error: ignoring #pragma gcc diagnostic`
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Workaround for Travis CI|AppVeyor + Catch: `suggest parentheses around comparison in operand of '=='`
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=parentheses")
  endif()

  find_package(Git REQUIRED)

  include(ExternalProject)
  ExternalProject_Add(
    catch
    PREFIX ${CMAKE_BINARY_DIR}/catch
    GIT_REPOSITORY https://github.com/philsquared/Catch.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON)
    ExternalProject_Get_Property(catch source_dir)
    set(CATCH_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "Path to include folder for Catch")
  
  file(GLOB_RECURSE geos_unit2_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
  add_executable(geos_unit2 ${geos_unit2_SOURCES})

  target_include_directories(geos_unit2 PRIVATE
    $<BUILD_INTERFACE:${CATCH_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)


  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(geos_unit2 GEOS)
 	else()
    target_link_libraries(geos_unit2 geos geos_c)
 	endif()

  add_test(geos_unit2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/geos_unit2)

  set(STATUS_RESULT "ON")
endif()

message(STATUS "${STATUS_MESSAGE} - ${STATUS_RESULT}")

#################################################################################
# Group source files for IDE source explorers (e.g. Visual Studio)
#################################################################################
GenerateSourceGroups(tests/unit2)
