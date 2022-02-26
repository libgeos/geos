---
title: "Testing"
draft: false
weight: 100
---

Testing executables are in `bin`

## Unit Tests

The GEOS unit tests are written using the [TUT](http://mrzechonek.github.io/tut-framework/) framework.
The test source files are under the `test/unit` directory.
The test group names are in the `group` declaration in each `XXXTest.cpp` file .

The entire set of unit tests is run by calling the `test_geos_unit` executable:
```
$ bin/test_geos_unit                  # Run all tests
```

Individual unit test classes can be run by calling `test_geos_unit` with a test group name:
```
$ bin/test_geos_unit capi::GEOSNode   # Run a single group of tests
```

Individual tests within a test class can be run by specifying the test number:
```
$ bin/test_geos_unit capi::GEOSNode 1
```

## XML Tests

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

## Performance Tests

There are performance tests which are built, but not run as part of the standard test suite.
These can be run from the command line:

```
$ bin/perf_iterated_buffer
```
