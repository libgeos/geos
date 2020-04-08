#################################################################################
#
# Copyright (C) 2019 Paul Ramsey <pramsey@cleverelephant.ca>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
#################################################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE astyle_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(astyle ${astyle_SOURCES})
if(MSVC)
  target_compile_options(astyle PRIVATE /source-charset:utf-8)
endif()
# message(STATUS "Enable AStyle")

