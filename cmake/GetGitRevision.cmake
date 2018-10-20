# Adapted from https://gitlab.kitware.com/cmake/cmake/blob/aec06dd4922187ce5346d20a9f0d53f01b6ce9fc/Source/CMakeVersionSource.cmake

# Try to identify the current development source version.
function(get_git_revision)
  cmake_parse_arguments("ARG" "" "HASH;HASH_LENGTH;DIRTY" "" "${ARGN}")

  if(EXISTS ${CMAKE_SOURCE_DIR}/.git/HEAD)
    find_program(GIT_EXECUTABLE NAMES git git.cmd)
    mark_as_advanced(GIT_EXECUTABLE)

    if(GIT_EXECUTABLE)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --verify -q --short=${ARG_HASH_LENGTH} HEAD
        OUTPUT_VARIABLE head
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
      if(head)
        execute_process(
          COMMAND ${GIT_EXECUTABLE} update-index -q --refresh
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          )
        execute_process(
          COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD -- --ignore-space-at-eol
          OUTPUT_VARIABLE dirty
          OUTPUT_STRIP_TRAILING_WHITESPACE
          WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          )
      endif()
    endif()
  endif()

  set(${ARG_HASH} "${head}" PARENT_SCOPE)

  if(dirty)
    set(${ARG_DIRTY} 1 PARENT_SCOPE)
  else()
    set("${ARG_DIRTY}" 0 PARENT_SCOPE)
  endif()
endfunction()
