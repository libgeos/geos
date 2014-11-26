#################################################################################
#
# GEOS C++ library build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

file(GLOB_RECURSE geos_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE geos_ALL_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h) # fix source_group issue

if(GEOS_ENABLE_MACOSX_FRAMEWORK)
  # OS X frameworks don't have static libs
  # also 1 binary, so include CAPI here
  # and, make name all caps

  set(geos_c_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../capi/geos_c.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../capi/geos_ts_c.cpp)

  add_library(GEOS SHARED ${geos_SOURCES} ${geos_c_SOURCES})

  math(EXPR CVERSION "${VERSION_MAJOR} + 1") 
 	# VERSION = current version, SOVERSION = compatibility version 
  set_target_properties(GEOS
    PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    VERSION "${CVERSION}.${VERSION_MINOR}.${VERSION_PATCH}" 
 	  SOVERSION ${CVERSION} 
    FRAMEWORK_VERSION ${VERSION_MAJOR}
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    MACOSX_FRAMEWORK_BUNDLE_VERSION "GEOS ${VERSION}"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.osgeo.geos)

  install(TARGETS GEOS FRAMEWORK DESTINATION .)

  if(GEOS_ENABLE_MACOSX_FRAMEWORK_UNIXCOMPAT)
    # Unix compatibility option, provides typical bin/lib/include folder
    # structure for framework-challenged projects

    set(GEOS_FWDIR "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/GEOS.framework")
    set(GEOS_FWDIR_VER "${GEOS_FWDIR}/Versions/${VERSION_MAJOR}")
    install(CODE "execute_process(COMMAND mkdir -p \"${GEOS_FWDIR_VER}/unix/bin\")")
    install(CODE "execute_process(COMMAND mkdir -p \"${GEOS_FWDIR_VER}/unix/include\")")
    install(CODE "execute_process(COMMAND mkdir -p \"${GEOS_FWDIR_VER}/unix/lib\")")
    install(CODE "execute_process(COMMAND ln -sfh ../../Headers \"${GEOS_FWDIR_VER}/unix/include/geos\")")
    install(CODE "execute_process(COMMAND ln -sf ../../Headers/geos_c.h \"${GEOS_FWDIR_VER}/unix/include/geos_c.h\")")
    install(CODE "execute_process(COMMAND ln -sf ../../GEOS \"${GEOS_FWDIR_VER}/unix/lib/libgeos.dylib\")")
    install(CODE "execute_process(COMMAND ln -sf ../../GEOS \"${GEOS_FWDIR_VER}/unix/lib/libgeos_c.dylib\")")
    install(CODE "execute_process(COMMAND ln -sfh Versions/Current/unix \"${GEOS_FWDIR}/unix\")")
  endif()

else()

  add_library(geos SHARED ${geos_SOURCES} ${geos_ALL_HEADERS})
  add_library(geos-static STATIC ${geos_SOURCES} ${geos_ALL_HEADERS})

  set_target_properties(geos
    PROPERTIES
    DEFINE_SYMBOL GEOS_DLL_EXPORT
    VERSION ${VERSION}
    CLEAN_DIRECT_OUTPUT 1)

  set_target_properties(geos-static
    PROPERTIES
    OUTPUT_NAME "geos"
    PREFIX "lib"
    CLEAN_DIRECT_OUTPUT 1)

endif()

# if(APPLE)
#   set_target_properties(geos
#     PROPERTIES
#     BUILD_WITH_INSTALL_RPATH TRUE
#     INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}")
# endif()

#################################################################################
# Installation
#################################################################################

if(NOT GEOS_ENABLE_MACOSX_FRAMEWORK)
  install(TARGETS geos geos-static
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
endif()

#################################################################################
# Group source files for IDE source explorers (e.g. Visual Studio)
#################################################################################
GenerateSourceGroups(src)
