---
title: "GEOS RFC 8 - Improve Coordinate Sequence API (WIP)"
date: 2021-10-04T14:21:00-07:00
draft: false
geekdocHidden: true
geekdocHiddenTocTree: false
---

|         |                                 |
| :------ | ------------------------------- |
| RFC 8   | Improve Coordinate Sequence API |
| Author  | Martin Davis                    |
| Contact | martin.davis@crunchydata.com    |
| Status  | In Discussion                   |

This document proposes to modify the Coordinate Sequence API to improve performance and adaptiveness.

These improvements are (likely to be) breaking changes to the C++ API.

These may require extensions to the C API to be externally available, but should not result in breaking the current C API.

## Background

The Coordinate Sequence API as it stands imposes a large cost on clients.

* It requires copying coordinate list structures one or more times
* It imposes the cost of a Z ordinate even if not required by the client
* (related) It is necessary to construct a full Geometry object just to pass a simple Point (e.g. for Point0In-Polygon)
* (related) Geometry objects for Point and Multi Point are very memory inefficient due to Point overhead

The API also has some functional limitations:

* does not support M values

Downstream projects which are feeling pain:

* PDAL - had to use custom Point-In-Polygon because of overhead in passing points to GEOS
* Shapely - there are several allocations required to marshall from NumPY to GEOS
* PostGIS - hopefully this will allow calling GEOS without copying out of LWGEOM structure

## Goals

* Allow using external coordinate list structures with no copying (except as needed by GEOS algorithms, e.g. removing repeated points)
* Prevent mutating of external coordinate structures
* Support XY, XYM, XYZ, XYZM

  * Coord Seq will need to know dimension of coordinates

* Support efficient input of Point data
* Optimized storage of Point and Multi Point data

## Ideas

**Memory-based Coordinate Sequence implementation**

* Class which contains pointer to memory block of coordinates, length, dimension
* Coordinate Sequence becomes a slimmed-down interface with accessors
* Will still provide setters, but use const to prevent unwanted modification
* How will coordinates be accessed?
   * By copying into stack-allocated object?  This would allow using a Coordinate with XYZM
   * By getX, getY and optional getZ, getM?  This requires rewriting some GEOS code to avoid copying coordinates

**Templates**

* problem: would templates pervade entire code base?
* does not allow dynamic adapting to external structures?

**Prior Art**

* C++ [string_view](https://www.modernescpp.com/index.php/c-17-avoid-copying-with-std-string-view)

## Tasks

1. Remove extraneous operations from CoordinateSequence (e.g. removeRepeatedPoints)
1. Create a MemoryBasedCoordinateSequence (better name?) which allows access to external blocks of memory
1. Review how Coordinates are accessed - is there a better way?
1. Review how this can provide XYZM capability?
