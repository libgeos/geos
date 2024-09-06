---
title: "Version 3.13.0"
type: posts
date: 2024-09-06T00:00:00
---

The 3.13 release of GEOS is now available [to download]({{< ref "/usage/download" >}}).

The headline features of this release are:

* A new approach to boolean predicates via [RelateNG](/posts/2024-08-13-relateng/)
  * Faster performance for many cases where a short circuit is available
  * Avoiding the full computation of a topology graph for every call
  * Ability to use a high speed "prepared" approach for the complete set of predicates as well as the relate matrix functions

* Initial support for ISO SQL/MM curve types
  * Create, read and write support for CircularString, CompoundCurve, CurvedPolygon, MultiCurve, MultiSurface

This release includes the following new features in the C API (and of course underlying changes to the C++ code to support these features):

  - GEOSGeom_createEmptyCircularString() creates a new empty CircularString
  - GEOSGeom_createCircularString(coordseq) creates a CircularString with the supplied coordinates
  - GEOSGeom_createEmptyCompoundCurve() creates a new empty CompoundCurve
  - GEOSGeom_createCompoundCurve(curves, ncurves) creates a CompoundCurve with the supplied components (start/end coordinates must match up)
  - GEOSGeom_createEmptyCurvePolygon() creates a new empty CurvePolygon
  - GEOSGeom_createCurvePolygon(shell, holes, nholes) creates a CurvePolygon with the supplied rings
  - GEOSPreparedRelate(prepgeom, geom) generates a [DE9IM](https://en.wikipedia.org/wiki/DE-9IM) intersection matrix for the geometry pair, where one input has been "prepared" for fast repeated calls
  - GEOSPreparedRelatePattern(prepgeom, geom, pattern) tests a pair of geometry against a known DE9IM pattern to see if the relationship is consistent with the pattern


<!--more-->

The full list of changes is as follows:

- **New things:**
  - Add classes for curved geometry types ([GH-1046](https://github.com/libgeos/geos/issues/1046), Dan Baston/German QGIS users group/Canton of Basel-Landschaft/Canton of Zug)
    - CircularString
    - CompoundCurve
    - CurvedPolygon
    - MultiCurve
    - MultiSurface
  - Support curved geometry types in WKT/WKB readers/writers ([GH-1046](https://github.com/libgeos/geos/issues/1046), [GH-1104](https://github.com/libgeos/geos/issues/1104), [GH-1106](https://github.com/libgeos/geos/issues/1106), Dan Baston)
  - 3D read and write support for GeoJSON ([GH-1150](https://github.com/libgeos/geos/issues/1150), Oreilles)
  - RelateNG ([GH-1052](https://github.com/locationtech/jts/pull/1052) Martin Davis, Paul Ramsey)
    - Rewrite of [boolean predicates and relate matrix calculations](https://lin-ear-th-inking.blogspot.com/2024/05/jts-topological-relationships-next.html)
    - "Prepared" mode now [available for all predicates](https://lin-ear-th-inking.blogspot.com/2024/05/relateng-performance.html) and relate matrix
    - CAPI functions GEOSPreparedRelate and GEOSPreparedRelatePattern expose new functionality
    - CAPI implementations of GEOSPreparedTouches, etc, that were previously defaulting
      into non-prepared implementations now default into the RelateNG prepared implementation
    - Prepared implementations for Intersects, Covers, still use the older implementations

- **Breaking Changes:**
  - Zero-length linestrings (eg LINESTRING(1 1, 1 1)) are now treated as equivalent to points (POINT(1 1)) in boolean predicates
  - CMake 3.15 or later is requried ([GH-1143](https://github.com/libgeos/geos/issues/1143), Mike Taves)

- **Fixes/Improvements:**
  - Add Angle::sinCosSnap to avoid small errors, e.g. with buffer operations ([GH-978](https://github.com/libgeos/geos/issues/978), Mike Taves)
  - WKTReader: Points with all-NaN coordinates are not considered empty anymore ([GH-927](https://github.com/libgeos/geos/issues/927), Casper van der Wel)
  - WKTWriter: Points with all-NaN coordinates are written as such ([GH-927](https://github.com/libgeos/geos/issues/927), Casper van der Wel)
  - ConvexHull: Performance improvement for larger geometries ([JTS-985](https://github.com/locationtech/jts/issues/985), Martin Davis)
  - Distance: Improve performance, especially for point-point distance ([GH-1067](https://github.com/libgeos/geos/issues/1067), Dan Baston)
  - Intersection: change to using DoubleDouble computation to improve robustness ([GH-937](https://github.com/libgeos/geos/issues/937), Martin Davis)
  - Fix LargestEmptyCircle to respect polygonal obstacles ([GH-939](https://github.com/libgeos/geos/issues/939), Martin Davis)
  - Fix WKTWriter to emit EMPTY elements in multi-geometries ([GH-952](https://github.com/libgeos/geos/issues/952), Mike Taves)
  - Fix IncrementalDelaunayTriangulator to ensure triangulation boundary is convex ([GH-953](https://github.com/libgeos/geos/issues/953), Martin Davis)
  - Fix PreparedLineStringDistance for lines within envelope and polygons ([GH-959](https://github.com/libgeos/geos/issues/959), Martin Davis)
  - Improve scale handling for PrecisionModel ([GH-956](https://github.com/libgeos/geos/issues/956), Martin Davis)
  - Fix error in CoordinateSequence::add when disallowing repeated points ([GH-963](https://github.com/libgeos/geos/issues/963), Dan Baston)
  - Fix WKTWriter::writeTrimmedNumber for big and small values ([GH-973](https://github.com/libgeos/geos/issues/973), Mike Taves)
  - Fix InteriorPointPoint to handle empty elements ([GH-977](https://github.com/libgeos/geos/issues/977), Martin Davis)
  - Fix TopologyPreservingSimplifier endpoint handling to avoid self-intersections ([GH-986](https://github.com/libgeos/geos/issues/986), Martin Davis)
  - Fix spatial predicates for MultiPoint with EMPTY ([GH-989](https://github.com/libgeos/geos/issues/989), Martin Davis)
  - Fix DiscreteHausdorffDistance for LinearRing ([GH-1000](https://github.com/libgeos/geos/issues/1000), Martin Davis)
  - Fix IsSimpleOp for MultiPoint with empty element ([GH-1005](https://github.com/libgeos/geos/issues/1005), Martin Davis)
  - Fix PreparedPolygonContains for GC with MultiPoint ([GH-1008](https://github.com/libgeos/geos/issues/1008), Martin Davis)
  - Fix reading WKT with EMPTY token with white space ([GH-1025](https://github.com/libgeos/geos/issues/1025), Mike Taves)
  - Fix buffer Inverted Ring Removal check ([GH-1056](https://github.com/libgeos/geos/issues/1056), Martin Davis)
  - Add PointLocation.isOnSegment and remove LineIntersector point methods ([GH-1083](https://github.com/libgeos/geos/issues/1083), Martin Davis)
  - Densify: Interpolate Z coordinates ([GH-1094](https://github.com/libgeos/geos/issues/1094))
  - GEOSLineSubstring: Fix crash on NaN length fractions ([GH-1088](https://github.com/libgeos/geos/issues/1088), Dan Baston)
  - MinimumClearance: Fix crash on NaN inputs ([GH-1082](https://github.com/libgeos/geos/issues/1082), Dan Baston)
  - Centroid: Fix crash on polygons with empty holes ([GH-1075](https://github.com/libgeos/geos/issues/1075), Dan Baston)
  - GEOSRelatePatternMatch: Fix crash on invalid DE-9IM pattern ([GH-1089](https://github.com/libgeos/geos/issues/1089), Dan Baston)
  - CoveragePolygonValidator: add section performance optimization ([GH-1099](https://github.com/libgeos/geos/issues/1099), Martin Davis)
  - TopologyPreservingSimplifier: fix to remove ring endpoints safely ([GH-1110](https://github.com/libgeos/geos/issues/1110), Martin Davis)
  - TopologyPreservingSimplifier: fix stack overflow on degenerate inputs ([GH-1113](https://github.com/libgeos/geos/issues/1113), Dan Baston)
  - DouglasPeuckerSimplifier: fix stack overflow on NaN tolerance ([GH-1114](https://github.com/libgeos/geos/issues/1114), Dan Baston)
  - GEOSConcaveHullOfPolygons: Avoid crash on zero-area input ([GH-1076](https://github.com/libgeos/geos/issues/1076), Dan Baston)


