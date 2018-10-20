################################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2018 Dan Baston, Vermont
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################

# TODO: Remove when autotools is dropped and configured headers
#       accept native CMake variables (eg. GEOS_MAJOR_VERSION)

# For compatibility with autotools, some of the .in files use @VERSION...@
# expressions instead of @GEOS_VERSION...@
# This function defines these variables before calling configure_file, without
# polluting the parent namespace.
function(geos_configure_file in out)
  set(VERSION ${GEOS_VERSION})
  set(VERSION_MAJOR ${GEOS_VERSION_MAJOR})
  set(VERSION_MINOR ${GEOS_VERSION_MINOR})
  set(VERSION_PATCH ${GEOS_VERSION_PATCH})

  configure_file(${in} ${out} @ONLY)
endfunction()



