---
title: "Version 3.14.0"
type: posts
date: 2025-08-21T00:00:00
---

The 3.14 release of GEOS is now available [to download]({{< ref "/usage/download" >}}).

The headline features of this release are:

* Coverage cleaning for polygonal coverages.
  * Remove small gaps and overlaps.
  * Control the tolerance of gap removal.
  * Choose between different gap removal heuristics.
    * Longest-border, max area, min area
* Clustering algorithms in the C API.
  * Exposing previously C++ only implementations into the C API for wider usage.
  * DBSCAN, Intersects, Envelope Intersects, and others
* Grid cell overlay with polygon.
  * Beyond the flood-fill, get actual proportions of each cell covered.

This release includes the following new features in the C API (and of course underlying changes to the C++ code to support these features):

  - GEOSCoordSeq_hasZ(coordseq) returns boolean
  - GEOSCoordSeq_hasM(coordseq) returns boolean
  - GEOSCoordSeq_setM(coordseq, index, value) sets the M ordinate on a point in a sequence
  - GEOSCoordSeq_getM(coordseq, index) reads the M ordinate on a point in a sequence
  - GEOSCoverageClean() and GEOSCoverageCleanWithParams(), with GEOSCoverageCleanParams for control over polygonal coverage cleaning
  - GEOSGridIntersectionFractions() for grid cell overlay
  - GEOSisSimpleDetail() to show where a linestring is not simple (self crossing points usually)
  - GEOSClusterDBSCAN(), GEOSClusterGeometryDistance(), GEOSClusterGeometryIntersects(), GEOSClusterEnvelopeDistance(), GEOSClusterEnvelopeIntersects() as CAPI clustering functions
  - GEOSGeoJSONWriter_setOutputDimension() for greater control of JSON output of 3 and 4 dimensional geometry.


<!--more-->

The full list of changes is as follows:

- **New things:**
  - Add clustering functions to C API ([GH-1154](https://github.com/libgeos/geos/issues/1154), Dan Baston)
  - Ported LineDissolver (Paul Ramsey)
  - Ported CoverageCleaner (Paul Ramsey)
  - Add GEOSGridIntersectionFractions to C API ([GH-1295](https://github.com/libgeos/geos/issues/1295), Dan Baston)
  - Add functions to interrupt processing in a specific thread/context ([GH-803](https://github.com/libgeos/geos/issues/803), Dan Baston)
  - Add "geos-targets.cmake" to build tree to allow building other software against
    GEOS build without installing first ([GH-1269](https://github.com/libgeos/geos/issues/1269), Dan Baston)
  - Add GEOSCoordSeq_hasZ, GEOSCoordSeq_hasM ([GH-1256](https://github.com/libgeos/geos/issues/1256), Aurele Ferotin)
  - Add GEOSCoordSeq_createWithDimensions, GEOSCoordSeq_setM, GEOSCoordSeq_getM ([GH-1246](https://github.com/libgeos/geos/issues/1246), Dan Baston)
  - Add GEOSGeoJSONWriter_setOutputDimension ([GH-1260](https://github.com/libgeos/geos/issues/1260), Aurele Ferotin)
  - Add GEOSGeom_transformXYZ ([GH-1157](https://github.com/libgeos/geos/issues/1157), Aurele Ferotin)
  - Add GEOSisSimpleDetail ([GH-1296](https://github.com/libgeos/geos/issues/1296), Dan Baston)

- **Breaking Changes:**
  - C++17 is now required ([GH-1144](https://github.com/libgeos/geos/issues/1144))
  - Stricter WKT parsing ([GH-1241](https://github.com/libgeos/geos/issues/1241), @freemine)
  - GEOSCoordSeq_setOrdinate returns an error if the sequence does not have the specified ordinate ([GH-1245](https://github.com/libgeos/geos/issues/1245), Dan Baston)

- **Fixes/Improvements:**
  - Fix ConcaveHullOfPolygons nested shell handling ([GH-1169](https://github.com/libgeos/geos/issues/1169), Martin Davis)
  - Fix RelateNG for computing IM for empty-nonempty cases (Martin Davis)
  - Fix TopologyPreservingSimplifier/TaggedLineString to avoid jumping components (JTS-1096, Martin Davis)
  - Fix WKTWriter for small precisions and with trim enabled ([GH-1199](https://github.com/libgeos/geos/issues/1199), Mike Taves)
  - Fix BufferOp to increase length of segments removed by heuristic ([GH-1200](https://github.com/libgeos/geos/issues/1200), Martin Davis)
  - Improve RelateNG performance for A/L cases in prepared predicates ([GH-1201](https://github.com/libgeos/geos/issues/1201), Martin Davis)
  - Improve OffsetCurve to handle mitre joins for polygons (Martin Davis)
  - Fix inscribed circle initialization ([GH-1225](https://github.com/libgeos/geos/issues/1225), Benoit Maurin)
  - Fix overlay heuristic for GeometryCollections with empty elements ([GH-1229](https://github.com/libgeos/geos/issues/1229), Martin Davis)
  - Add ring buffer hole removal heuristic ([GH-1233](https://github.com/libgeos/geos/issues/1233), Martin Davis)
  - Fix buffer element erosion for negative distance and remove overlay deps ([GH-1239](https://github.com/libgeos/geos/issues/1239), Martin Davis)
  - Fix OverlayNG coordinate dimension handling for EMPTY geometries ([GH-1258](https://github.com/libgeos/geos/issues/1258), Martin Davis)
  - Fix DepthSegment comparison logic (really this time) ([GH-1266](https://github.com/libgeos/geos/issues/1266), Martin Davis)
  - Change CoverageGapFinder to return polygons (Martin Davis)
  - Update DiscreteFrechetDistance to new algorithm ([GH-1274](https://github.com/libgeos/geos/issues/1274), Paul Ramsey)


