cmake_minimum_required(VERSION 3.15)
project(CPP_APP LANGUAGES CXX)

find_package(GEOS REQUIRED CONFIG)

include(CMakePrintHelpers)
cmake_print_properties(
  TARGETS GEOS::geos
  PROPERTIES
    LOCATION
    INTERFACE_INCLUDE_DIRECTORIES
    INTERFACE_LINK_LIBRARIES
    INTERFACE_COMPILE_FEATURES
)

add_executable(cpp_app cpp_app.cpp)
target_link_libraries(cpp_app PRIVATE GEOS::geos)
target_compile_features(cpp_app PRIVATE cxx_std_17)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/..)
include(common)
add_test_length(cpp_app)
add_test_libpath(cpp_app libgeos)
add_test_version(cpp_app)
