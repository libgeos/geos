#################################################################################
# $Id: CMakeLists.txt 3375 2011-05-22 23:34:04Z mloskot $
#
# GEOS tools (geos-config) build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

if(UNIX)

  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK AND GEOS_ENABLE_MACOSX_FRAMEWORK_UNIXCOMPAT)
    set(installdest GEOS.framework/Versions/${VERSION_MAJOR}/unix/bin)
    set(prefix ${CMAKE_INSTALL_PREFIX}/GEOS.framework/Versions/${VERSION_MAJOR}/unix)
  elseif(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    # just a dummy so installdest not set
    # want geos-config only for Mac unix build or framework+unixcompat
  else()
    set(installdest bin)
    set(prefix ${CMAKE_INSTALL_PREFIX})
  endif()

  set(exec_prefix ${prefix}/bin)
  set(libdir ${prefix}/lib)
  
  if(installdest)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/geos-config.in
                   ${CMAKE_CURRENT_BINARY_DIR}/geos-config @ONLY)
  
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos-config
      DESTINATION ${installdest}
      PERMISSIONS
      OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

endif()
