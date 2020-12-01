cmake_minimum_required(VERSION 3.8)
project(test_cpp LANGUAGES CXX)

find_package(GEOS REQUIRED)

# Show some target properties
get_cmake_property(_variableNames VARIABLES)
list(SORT _variableNames)
foreach(_variableName ${_variableNames})
  string(REGEX MATCH "^GEOS_" _matched ${_variableName})
  if(NOT ${_matched} STREQUAL "")
    message(STATUS "${_variableName}=${${_variableName}}")
  endif()
endforeach()

add_executable(test_cpp test_cpp.cpp)
target_link_libraries(test_cpp GEOS::geos)

include(CTest)

if(APPLE)
  set(LDD_CL "otool -L")
  set(EXPECTED_LDD_CL_OUT "@rpath/libgeos")
elseif(UNIX)
  set(LDD_CL "ldd")
  set(EXPECTED_LDD_CL_OUT "${CMAKE_PREFIX_PATH}/lib/libgeos")
endif()

if(LDD_CL)
  add_test(NAME test_ldd
    COMMAND sh -c "${LDD_CL} ${CMAKE_BINARY_DIR}/test_cpp | grep geos")
  set_tests_properties(test_ldd PROPERTIES
    PASS_REGULAR_EXPRESSION ${EXPECTED_LDD_CL_OUT}
  )
else()
  add_test(NAME test_ldd COMMAND test_cpp)
  set_tests_properties(test_ldd PROPERTIES SKIP_RETURN_CODE 0)
endif()

add_test(NAME test_version COMMAND test_cpp)
set_tests_properties(test_version PROPERTIES
  PASS_REGULAR_EXPRESSION "${GEOS_VERSION}"
)
