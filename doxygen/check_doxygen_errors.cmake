################################################################################
# Part of CMake configuration for GEOS
#
# Script checking the doxygen logfile for errors and warnings, which are not
# accepted. Throws FATAL_ERROR if found.
#
# Copyright (C) 2019 Nicklas Larsson <n_larsson@yahoo.com>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################

file(TO_NATIVE_PATH ${DOXYGEN_LOGFILE} DOXYGEN_LOGFILE)

if(EXISTS "${DOXYGEN_LOGFILE}")
  set(ERRORS "")
  file(READ "${DOXYGEN_LOGFILE}" LOGFILE)

  # convert file content to list
  string(REGEX REPLACE "\n$" "" LOGFILE "${LOGFILE}")
  string(REGEX REPLACE ";" "\\\\;" LOGFILE "${LOGFILE}")
  string(REGEX REPLACE "\n" ";" LOGFILE "${LOGFILE}")

  # let's not forget non-fatal warnings
  list(LENGTH LOGFILE NUM_WARNINGS)
  if(NUM_WARNINGS GREATER 0)
    message(STATUS
      "Doxygen issued ${NUM_WARNINGS} warning(s), see ${DOXYGEN_LOGFILE}")
  endif()

  foreach(LINE ${LOGFILE})
    string(REGEX MATCH
      ".*(not documented|ignoring unsupported tag).*" IGNORE ${LINE})
    if("${IGNORE}" STREQUAL "")
      list(APPEND ERRORS ${LINE})
    endif()
  endforeach()

  if(NOT "${ERRORS}" STREQUAL "")
    # convert list to string
    string(REGEX REPLACE ";" "\n" ERROR_MSG "${ERRORS}")
    message(FATAL_ERROR "${ERROR_MSG}")
  endif()

  unset(ERRORS)
endif()

message(STATUS "Doxygen documentation is OK")
