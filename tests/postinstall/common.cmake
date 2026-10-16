# Common CMake macros for post-install tests

# Note that the environment variable 'library_type' is considered

include(CTest)

macro(add_test_libpath EXE LIBNAME)
  set(EXEPATH ${CMAKE_BINARY_DIR}/${EXE})
  string(SUBSTRING ${LIBNAME} 0 3 LIBNAME_PREFIX)
  if(NOT LIBNAME_PREFIX STREQUAL "lib")
    message(SEND_ERROR
      "Expected LIBNAME '${LIBNAME}' to start with 'lib'; "
      "found '${LIBNAME_PREFIX}'")
  endif()
  set(HAS_TOOL TRUE)
  if(APPLE)
    set(EXPECTED_SUBSTR "${CMAKE_PREFIX_PATH}/lib")
  elseif(UNIX)
    set(EXPECTED_SUBSTR "${CMAKE_PREFIX_PATH}/lib/${LIBNAME}\.")
  elseif(CMAKE_GENERATOR STREQUAL "MSYS Makefiles")
    # Convert to Unix-style path
    execute_process(
      COMMAND cygpath -u "${CMAKE_PREFIX_PATH}/bin/${LIBNAME}\."
        OUTPUT_VARIABLE EXPECTED_SUBSTR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
  else()
    set(HAS_TOOL FALSE)
  endif()
  if(HAS_TOOL)
    if(APPLE)
      add_test(NAME test_libpath
        COMMAND sh -c "otool -l ${EXEPATH} | grep -m1 \"${EXPECTED_SUBSTR}\"")
    else()
      add_test(NAME test_libpath
        COMMAND sh -c "ldd ${EXEPATH} | grep -m1 ${LIBNAME}")
    endif()
    if($ENV{library_type} STREQUAL "static")
      set_tests_properties(test_libpath PROPERTIES
        PASS_REGULAR_EXPRESSION "^$"
        FAIL_REGULAR_EXPRESSION "${EXPECTED_SUBSTR};not found"
      )
    else()
      set_tests_properties(test_libpath PROPERTIES
        PASS_REGULAR_EXPRESSION "${EXPECTED_SUBSTR}"
        FAIL_REGULAR_EXPRESSION "not found"
      )
    endif()
  else()
    add_test(NAME test_libpath COMMAND ${EXEPATH})
    set_tests_properties(test_libpath PROPERTIES SKIP_RETURN_CODE 1)
  endif()
endmacro()

macro(add_test_length EXE)
  set(EXEPATH ${CMAKE_BINARY_DIR}/${EXE})
  add_test(NAME test_length COMMAND ${EXEPATH} -l)
  set_tests_properties(test_length PROPERTIES
    PASS_REGULAR_EXPRESSION "42\\.0"
  )
endmacro()

macro(add_test_version EXE)
  set(EXEPATH ${CMAKE_BINARY_DIR}/${EXE})
  add_test(NAME test_version COMMAND ${EXEPATH} -v)
  set_tests_properties(test_version PROPERTIES
    PASS_REGULAR_EXPRESSION "${GEOS_VERSION}"
  )
endmacro()
