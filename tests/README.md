GEOS Testing
============

## Build locations

Paths are given relative to GEOS root.
Note some executables have different names.

### CMake

* Executables are in build directory (usually `../build`)
  * `../build/bin/test_geos_unit`
  * `../build/bin/test_xmltester`

## Unit tests

    geos_unit [class] [test number]

Run all unit tests

    geos_unit

Run single unit test class

    geos_unit capi::GEOSNode

Run single test within unit test class

    geos_unit capi::GEOSNode 1

## XML Tests

    xmltester [options] test-file ...

### Options

* `-v` - show test case summary with time
* `-v -v` - show test case info including dump of geometry

CMake - Run test file

    bin/test_xmltester ../geos/tests/xmltester/tests/general/TestBoundary.xml

## Adding New Tests

* In order for new unit tests to be picked up by the test harness, they **must** be named using the name pattern "\*Test.cpp".
* In order for new XML tests to be picked up, they must reside in a directory under the `xmltester/` directory and have a `.xml` file suffix.


