#################################################################################
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

#
# Configure geos-config
#
function(configure_install_geos_config)
  string(REPLACE " " "\\ " prefix ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix "$\{prefix\}")
  if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")
  else()
    set(includedir "$\{prefix\}/${CMAKE_INSTALL_INCLUDEDIR}")
  endif()
  if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
    set(libdir "${CMAKE_INSTALL_LIBDIR}")
  else()
    set(libdir "$\{exec_prefix\}/${CMAKE_INSTALL_LIBDIR}")
  endif()
  set(VERSION_RELEASE ${GEOS_VERSION_MAJOR})
  set(VERSION ${GEOS_VERSION})

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/geos-config.in
    ${CMAKE_CURRENT_BINARY_DIR}/geos-config
    @ONLY NEWLINE_STYLE UNIX)

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/geos-config
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS
      OWNER_READ OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE)
endfunction()

#
# Configure pkg-config file geos.pc
#
function(configure_install_geos_pc)
  string(REPLACE " " "\\ " prefix ${CMAKE_INSTALL_PREFIX})
  set(exec_prefix "$\{prefix\}")
  if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")
  else()
    set(includedir "$\{prefix\}/${CMAKE_INSTALL_INCLUDEDIR}")
  endif()
  if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
    set(libdir "${CMAKE_INSTALL_LIBDIR}")
  else()
    set(libdir "$\{exec_prefix\}/${CMAKE_INSTALL_LIBDIR}")
  endif()
  set(VERSION ${GEOS_VERSION})
  set(EXTRA_LIBS "-lstdc++")
  if(HAVE_LIBM)
    list(APPEND EXTRA_LIBS "-lm")
  endif()
  list(JOIN EXTRA_LIBS " " EXTRA_LIBS)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/geos.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/geos.pc
    @ONLY)

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/geos.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endfunction()

if(NOT MSVC)
  configure_install_geos_config()
  configure_install_geos_pc()
endif()

option(BUILD_ASTYLE "Build astyle (Artistic Style) tool" OFF)

message(STATUS "GEOS: Build astyle ${BUILD_ASTYLE}")
if(BUILD_ASTYLE)
  add_subdirectory(astyle)
endif()

# if(UNIX OR MINGW)

#   if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK AND GEOS_ENABLE_MACOSX_FRAMEWORK_UNIXCOMPAT)
#     set(installdest GEOS.framework/Versions/${VERSION_MAJOR}/unix/bin)
#     set(prefix ${CMAKE_INSTALL_PREFIX}/GEOS.framework/Versions/${VERSION_MAJOR}/unix)
#   elseif(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
#     # just a dummy so installdest not set
#     # want geos-config only for Mac unix build or framework+unixcompat
#   else()
#     set(installdest bin)
#     set(prefix ${CMAKE_INSTALL_PREFIX})
#   endif()

#   set(exec_prefix ${prefix}/bin)
#   set(libdir ${prefix}/lib)

#   if(installdest)
#     configure_file(${CMAKE_CURRENT_SOURCE_DIR}/geos-config.in
#                    ${CMAKE_CURRENT_BINARY_DIR}/geos-config @ONLY)

#     install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos-config
#       DESTINATION ${installdest}
#       PERMISSIONS
#       OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
# endif()

# endif()

