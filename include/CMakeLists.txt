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

#################################################################################
# Install/Uninstall
#################################################################################

# There is a builtin cmake way to get headers in the right location
# for OS X framework, but it's not practical this complex tree

if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
  set(installdest GEOS.framework/Versions/${VERSION_MAJOR}/Headers)
else()
  set(installdest include/geos)
endif()


install(FILES geos.h
  DESTINATION ${installdest})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos/platform.h
  DESTINATION ${installdest})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos/version.h
  DESTINATION ${installdest})

install(DIRECTORY geos/
  DESTINATION ${installdest}
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "*.inl"
  PATTERN ".svn" EXCLUDE)

# Some post-processing for the framework
# luckily GEOS uses header subdir already, just need to change case
if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
  install(CODE 
    "execute_process(COMMAND find \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${installdest}\" -name *.* -print0 
    COMMAND xargs -0 sed -E -i \"\" \"s,# *include[[:space:]]+<geos/,#include <GEOS/,g\")")
  install(CODE 
    "execute_process(COMMAND ln -sfh Versions/Current/Headers \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/GEOS.framework/Headers\")")
endif()
