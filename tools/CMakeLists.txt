#################################################################################
# $Id$
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

  # Autoconf compatibility variables to use the same script source.
  set(prefix ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix ${CMAKE_INSTALL_PREFIX}/bin)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/geos-config.in
    ${CMAKE_CURRENT_BINARY_DIR}/geos-config @ONLY)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos-config
    DESTINATION bin/
    PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

endif()
