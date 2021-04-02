cmake_minimum_required(VERSION 3.8)
project(test_c LANGUAGES C)

find_package(GEOS REQUIRED)

add_executable(test_c test_c.c)
target_link_libraries(test_c GEOS::geos_c)

include(CTest)

if(APPLE)
  set(LDD_CL "otool -L")
  set(EXPECTED_LDD_CL_OUT "@rpath/libgeos")
elseif(UNIX)
  set(LDD_CL "ldd")
  set(EXPECTED_LDD_CL_OUT "${CMAKE_PREFIX_PATH}/lib/libgeos")
elseif(CMAKE_GENERATOR STREQUAL "MSYS Makefiles")
  set(LDD_CL "ldd")
  # Convert to Unix-style path
  execute_process(
    COMMAND cygpath -u ${CMAKE_PREFIX_PATH}/bin/libgeos
	OUTPUT_VARIABLE EXPECTED_LDD_CL_OUT
	OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if(LDD_CL)
  add_test(NAME test_ldd
    COMMAND sh -c "${LDD_CL} ${CMAKE_BINARY_DIR}/test_c | grep geos")
  set_tests_properties(test_ldd PROPERTIES
    PASS_REGULAR_EXPRESSION ${EXPECTED_LDD_CL_OUT}
  )
else()
  add_test(NAME test_ldd COMMAND test_c)
  set_tests_properties(test_ldd PROPERTIES SKIP_RETURN_CODE 0)
endif()

add_test(NAME test_version COMMAND test_c)
set_tests_properties(test_version PROPERTIES
  PASS_REGULAR_EXPRESSION "${GEOS_VERSION}"
)
