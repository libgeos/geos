# GeosOp User Guide

`geosop` is a CLI (command-line interface) for GEOS.
It can be used to:

* Run GEOS operations on one or many geometries
* Output geometry resuls in various formats (WKT and WKB)
* Convert between WKT and WKB
* Time the performance of operations
* Check for memory leaks in operations
* Check the semantics of GEOS operations

## Features

* Read list of geometries from a file (WKT or WKB)
* Read geometries from stdin (WKT or WKB)
* Read geometry from command-line literal (WKT or WKB)
* Input format is WKT or WKB
* Apply a limit and offset (TBD) to the input geometries
* collect input geometries into a GeometryCollection (for aggregate operations)
* Execute a GEOS operation on each geometry
* TBD: Execute a GEOS operation on each geometry for a list of different arguments
* Explode result collections into individual geometries
* Output result as text, WKT or WKB
* Time the overall and individual performance of each operation

## Usage
```
  geosop [OPTION...] opName opArg

  -a arg               source for A geometries (WKT, WKB, file, stdin, stdin.wkb)
  -b arg               source for B geometries (WKT, WKB, file, stdin, stdin.wkb)
      --alimit arg     Limit nunber of A geometries read
  -c, --collect        Collect input into single geometry
  -e, --explode        Explode results into component geometries
  -f, --format arg     Output format (wkt, wkb or txt)
  -h, --help           Print help
  -p, --precision arg  Sets number of decimal places in output coordinates
  -r, --repeat arg     Repeat operation N times
  -t, --time           Print execution time
  -v, --verbose        Verbose output
```

## Examples

**Note: TBD = To Be Developed**

* Print usage instructions

    `geosop`

* Print usage instructions and list of available operations

    `geosop --help`

* Read a file of geometries in WKT and output them as WKB

    `geosop -a geoms.wkt -f wkb`

* Compute the area of geometries in a WKT file and output them as text

    `geosop -a geoms.wkt --format=txt area`

* Compute the centroids of geometries in a WKT file and output them as WKT

    `geosop -a geoms.wkt -f wkt centroid`

* Compute an operation on a list of geometries and output only geometry metrics and timing

    `geosop -v -a geoms.wkt isValid`

* Validate geometries from a WKT file, limiting the number of geometries read

    `geosop -a geoms.wkt --alimit 100 -f txt isValid`

* Compute the buffer with distance 10 of WKB geometries and output as WKT

    `geosop -a geoms.wkb -f wkt buffer 10`

* Compute the unary union of a set of WKT geometries and output as WKB

    `geosop -a geoms.wkt --collect -f wkb unaryUnion`

* Compute the buffer of a WKB literal and output as WKT

    `geosop -a 000000000140240000000000004024000000000000 -f wkt buffer 10`

* Polygonize lines and output the individual result polygons as WKT

    `geosop -a "MULTILINESTRING ((200 100, 100 100, 200 200), (200 200, 200 100), (200 200, 300 100, 200 100))" -v -e -f wkt polygonize`

* Read geometries from a WKT file on stdin and output as WKB

    `geosop -a - -f wkb`

* Read geometries from a WKB file on stdin and output as WKT

    `geosop -a stdin.wkb -f wkt`

* Run a sequence of operations using a pipe

    `geosop -a "LINESTRING (10 10, 20 20)" -f wkt buffer 10 | geosop -a - -f wkt envelope`

* Compute the union of two geometries in WKT and WKB and output as WKT

    `geosop -a some-geom.wkt -b some-other-geom.wkb -f wkt union`

* TBD: Compute the buffer of a WKT literal for multiple distances

    `geosop -a "MULTIPOINT ( (0 0), (10 10) )" -f wkt buffer 1,2,3,4`

* TBD: Compute the buffer of a WKT literal and output as WKB, with SRID set to 4326

    `geosop -a "POINT (10 10)" --srid=4326 -f wkb buffer 10`



## Future Ideas

* `--explode` outputs individual elements of result geometry collections
* `--sort [ asc | desc ]` sorts output geometries by value of operation
* `--where [eq | gt |ge | lt | le ]:val` filters result geometries by value of operation
* `--limit N` applies limit to output (used with sorting)
* `--repeat N` repeats operation N times, for more accurate timing
* `--srid s` sets SRID to s, for WKB output
* support more operation arguments (e.g. to provide translate X Y)
