# GeosOp User Guide

`geosop` is a CLI (command-line interface) for GEOS.
It can be used to:

* Run GEOS operations on one or many geometries
* Output geometry results in various formats (WKT, WKB and GeoJSON)
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

  -a arg               source for A geometries (WKT, WKB, file, stdin,
                       stdin.wkb)
  -b arg               source for B geometries (WKT, WKB, file, stdin,
                       stdin.wkb)
  -l, --limita arg     Limit number of A geometries read
  -o, --offseta arg    Skip reading first N geometries of A
  -c, --collect        Collect input into single geometry (automatic for AGG
                       ops)
  -e, --explode        Explode results into component geometries
  -f, --format arg     Output format (wkt, wkb, txt or geojson)
  -p, --precision arg  Set number of decimal places in output coordinates
  -q, --quiet          Disable result output
  -r, --repeat arg     Repeat operation N times
  -t, --time           Print execution time
  -v, --verbose        Verbose output
  -h, --help           Print help
```

## Examples

* Print usage instructions

    `geosop`

* Print usage instructions and list of available operations

    `geosop --help`

* Read a file of geometries in WKT and output them as WKB

    `geosop -a geoms.wkt -f wkb`

* Compute the area of geometries in a WKT file and output them as text

    `geosop -a geoms.wkt area`

* Compute the centroids of geometries in a WKT file and output them as WKT

    `geosop -a geoms.wkt centroid`

* Compute an operation on a list of geometries and output only geometry metrics and timing

    `geosop -a geoms.wkt -v -q isValid`

* Validate geometries from a WKT file, reading only 5 and skipping the first 10

    `geosop -a geoms.wkt --limita 5 --offseta 10 isValid`

* Compute the buffer with distance 10 of WKB geometries and output as WKT

    `geosop -a geoms.wkb -f wkt buffer 10`

* Compute the unary union of a set of WKT geometries and output as WKB
  * `unaryUnion` is an aggregate operation, so automatically collects all input geometries

    `geosop -a geoms.wkt -f wkb unaryUnion`

* Compute the buffer of a WKB literal and output as GeoJSON

    `geosop -a 000000000140240000000000004024000000000000 -f geojson buffer 10`

* Polygonize lines and output the individual result polygons as WKT

    `geosop -a "MULTILINESTRING ((200 100, 100 100, 200 200), (200 200, 200 100), (200 200, 300 100, 200 100))" -v -e polygonize`

* Read geometries from a WKT file on stdin and output as WKB

    `geosop -a - -f wkb`

* Read geometries from a WKB file on stdin and output as WKT

    `geosop -a stdin.wkb`

* Run a sequence of operations using a pipe

    `geosop -a "LINESTRING (10 10, 20 20)" buffer 10 | geosop -a - envelope`

* Compute the union of two geometries in WKT and WKB and output as WKT

    `geosop -a some-geom.wkt -b some-other-geom.wkb union`

* TBD: Compute the buffer of a WKT literal for multiple distances

    `geosop -a "MULTIPOINT ( (0 0), (10 10) )" buffer 1,2,3,4`

* TBD: Compute the buffer of a WKT literal and output as WKB, with SRID set to 4326

    `geosop -a "POINT (10 10)" --srid=4326 -f wkb buffer 10`



## Future Ideas

* `--sort [ asc | desc ]` sorts output geometries by value of operation
* `--where [eq | gt |ge | lt | le ]:val` filters result geometries by value of operation
* `--limit N` applies limit to output (used with sorting)
* `--srid s` sets SRID to s, for WKB output
