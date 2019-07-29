GEOS Testing
============

## Build locations

Paths are given relative to GEOS root.
Note some executables have different names.

### Autotools

* Executables are in-tree
  * `tests/unit/geos_unit`
  * `tests/xmltests/XMLtester`

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

Run test witin unit test class

    geos_unit capi::GEOSNode 1

## XML Tests

    xmltester [options] test-file ...

### Options

* `-v` - show test case summary with time
* `-v -v` - show test case info including dump of geometry

CMake - Run test file

    bin/test_xmltester ../geos/tests/xmltester/tests/general/TestBoundary.xml


