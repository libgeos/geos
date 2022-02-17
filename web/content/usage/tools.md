---
title: "Tools"
draft: false
weight: 100
---

## Testing Tools

Testing executables are in `bin`

### Unit testing

Individual test classes within the `test_geos_unit` suite can be run by calling the `test_geos_unit` executable.
The test group name is found in the cpp file in the `group` declaration.

```
$ bin/test_geos_unit capi::GEOSNode
```

Individual tests within a test class can be run by specifying the test number:

```
$ bin/test_geos_unit capi::GEOSNode 1
```

### XML Tests

The XML test suite is a portable set of unit tests which can be run in both GEOS and JTS.
They are located in `../geos/tests/xmltester/tests`.
XML tests can be run using `test_xmltester`:

```
bin/test_xmltester ../geos/tests/xmltester/tests/robust/overlay/TestOverlay-geos-837.xml
```

The XML Tester supports the following options:

```
Usage: bin/test_xmltester [options] <test> [<test> ...]
Options:
 -v                  Verbose mode (multiple -v increment verbosity)
--test-valid-output  Test output validity
--test-valid-input   Test input validity
--sql-output         Produce SQL output
--wkb-output         Print Geometries as HEXWKB
```

### Additional Testing Tools

There are additional tests (typically for performance) which are built, but not run as part of the standard test suite.
These can be run from the command line:

```
$ bin/perf_iterated_buffer
```

## GeosOp

`geosop` is a CLI (command-line interface) for GEOS. It can be used to:

* Run GEOS operations on one or many geometries
* Output geometry resuls in various formats (WKT and WKB)
* Convert between WKT and WKB
* Time the performance of operations
* Check for memory leaks in operations
* Check the semantics of GEOS operations

For more information see the [README](https://github.com/libgeos/geos/tree/main/util/geosop).

### Usage

The `geosop` executable is in `bin`.
*
Print usage instructions
  ```
  bin/geosop
  ```
* Print usage instructions and list of available operations
  ```
  bin/geosop --help
  ```
* Read a file of geometries in WKT and output them as WKB
  ```
  bin/geosop -a geoms.wkt -f wkb
  ```
* Compute the area of geometries in a WKT file and output them as text
  ```
  bin/geosop -a geoms.wkt --format=txt area
  ```
* Compute the centroids of geometries in a WKT file and output them as WKT
  ```
  bin/geosop -a geoms.wkt -f wkt centroid
  ```

## JTS TestBuilder

The **JTS TestBuilder** can be useful for creating, visualizing and processing GEOS geometry.
Because GEOS and JTS are so closely related it can also be used as a way of verifying GEOS behaviour.
For more information see the [JTS doc](https://github.com/locationtech/jts/blob/master/doc/JTSTestBuilder.md).
