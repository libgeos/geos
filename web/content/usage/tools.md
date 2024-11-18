---
title: "Tools"
draft: false
weight: 100
---

{{< toc >}}

## GeosOp

`geosop` is a CLI (command-line interface) for GEOS. It can be used to:

* Run GEOS operations on one or many geometries
* Output geometry results in various formats (WKT and WKB)
* Convert between WKT and WKB
* Time the performance of operations
* Check for memory leaks in operations
* Check the semantics of GEOS operations

For more information see the [README](https://github.com/libgeos/geos/tree/main/util/geosop).

### Usage

The `geosop` executable is in `bin`.

* Print usage instructions
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
