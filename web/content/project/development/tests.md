---
title: "Testing"
date: 2021-10-04T14:21:00-07:00
draft: false
weight: 20
---

Testing executables are in `bin`

* See also the `ctest` [documentation](/usage/download/#testing).

### Unit Tests

The GEOS unit tests are written using the [TUT](http://mrzechonek.github.io/tut-framework/) framework.
There are tests for both the C++ code and the C API.
The test source files are in the `test/unit` directory.
The test **group names** are in the `group` declaration in each `XXXTest.cpp` file.
By convention the test group name is based on the test file path and name.

The entire set of unit tests is run by calling the `test_geos_unit` executable:
```
$ bin/test_geos_unit                  # Run all tests
```

The unit tests groups can be listed:
```
$ bin/test_geos_unit --list           # List all test groups
```

Individual unit test groups can be run by calling `test_geos_unit` with the test group name:
```
$ bin/test_geos_unit capi::GEOSBuffer   # Run a single group of tests
```

Individual tests within a test group can be run by specifying the group name and test number:
```
$ bin/test_geos_unit capi::GEOSBuffer 1
```

### XML Tests

The XML test suite is a set of portable, declarative tests which can be run in both GEOS and JTS.
They are located in `../geos/tests/xmltester/tests`.
XML tests can be run using `test_xmltester`:

```
bin/test_xmltester ../geos/tests/xmltester/tests/robust/overlay/TestOverlay-geos-837.xml
```

The XML Tester provides the following options:

```
Usage: bin/test_xmltester [options] <test> [<test> ...]
Options:
 -v                  Verbose mode (multiple -v increment verbosity)
--test-valid-output  Test output validity
--test-valid-input   Test input validity
--sql-output         Produce SQL output
--wkb-output         Print Geometries as HEXWKB
```

### Performance Tests

There are performance tests which are built, but not run as part of the standard test suite.
These can be run from the command line:

```
$ bin/perf_iterated_buffer
```
