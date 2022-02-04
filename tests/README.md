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

    bin/test_geos_unit [name] [test number]
    
The test name is the **tut group name** found in the unit test file on a line begining `group` (e.g. like [this](https://github.com/libgeos/geos/blob/main/tests/unit/operation/buffer/BufferOpTest.cpp#L55).

### Run all unit tests

    bin/test_geos_unit

### Run single unit test class

    bin/test_geos_unit capi::GEOSNode

### Run single test within unit test class

    bin/test_geos_unit capi::GEOSNode 1

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


