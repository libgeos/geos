#################################################################################
#
# GEOS C library build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

if(WIN32)
    add_definitions("-DGEOS_DLL_EXPORT=1")
endif()

set(geos_c_SOURCES
  geos_c.cpp
  geos_ts_c.cpp)

file(GLOB geos_capi_HEADERS ${CMAKE_BINARY_DIR}/capi/*.h) # fix source_group issue

if(NOT GEOS_ENABLE_MACOSX_FRAMEWORK) 
  # if building OS X framework, CAPI built into C++ library 
  add_library(geos_c SHARED ${geos_c_SOURCES}) 

  target_link_libraries(geos_c geos)

  if (WIN32)
    set_target_properties(geos_c
      PROPERTIES
      VERSION ${CAPI_VERSION}
      CLEAN_DIRECT_OUTPUT 1)
  else()
    set_target_properties(geos_c
      PROPERTIES
      VERSION ${CAPI_VERSION}
      SOVERSION ${CAPI_SOVERSION}
      CLEAN_DIRECT_OUTPUT 1)
  endif()

endif()

#################################################################################
# Installation
#################################################################################

if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos_c.h
    DESTINATION GEOS.framework/Versions/${VERSION_MAJOR}/Headers)
  install(CODE "execute_process(COMMAND sed -E -i \"\" \"s,# *include[[:space:]]+<geos/,#include <GEOS/,g\" \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/GEOS.framework/Versions/${VERSION_MAJOR}/Headers/geos_c.h\")")
else()
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos_c.h
    DESTINATION include)
endif()

if(NOT GEOS_ENABLE_MACOSX_FRAMEWORK) 
  install(TARGETS geos_c
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif()

#################################################################################
# Group source files for IDE source explorers (e.g. Visual Studio)
#################################################################################

source_group("Header Files" FILES ${geos_capi_HEADERS})
