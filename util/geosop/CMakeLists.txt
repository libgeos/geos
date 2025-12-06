################################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2020 Martin Davis
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################

add_executable(geosop
  GeosOp.cpp
  GeometryOp.cpp
  )

target_link_libraries(geosop PRIVATE geos geos_c)

if(NOT HAVE_FENV_H)
  target_compile_definitions(geosop PRIVATE MISSING_FENV)
endif()

install(TARGETS geosop
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_SHARED_LIBS)
  if(NOT DEFINED CMAKE_INSTALL_RPATH)
    # Use relative rpath
    if(APPLE)
      set_target_properties(geosop PROPERTIES
        INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
    else()
      set_target_properties(geosop PROPERTIES
        INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    endif()
  endif()
endif()


