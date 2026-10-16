cmake_minimum_required(VERSION 3.15)
project(C_APP LANGUAGES C)

find_package(GEOS REQUIRED CONFIG)

include(CMakePrintHelpers)
cmake_print_properties(
  TARGETS GEOS::geos_c
  PROPERTIES
    LOCATION
    INTERFACE_INCLUDE_DIRECTORIES
    INTERFACE_LINK_LIBRARIES
    INTERFACE_COMPILE_FEATURES
)

add_executable(c_app c_app.c)
target_link_libraries(c_app PRIVATE GEOS::geos_c)

get_target_property(GEOSLIB_LOCATION GEOS::geos_c LOCATION)
if(GEOSLIB_LOCATION MATCHES ".*\.(lib|a)$")
  # Used for static linking (is there a better way?)
  enable_language(CXX)
  set_target_properties(c_app PROPERTIES LINKER_LANGUAGE CXX)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/..)
include(common)
add_test_length(c_app)
add_test_libpath(c_app libgeos_c)
add_test_version(c_app)
