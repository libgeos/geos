---
title: "Version 3.12.0"
type: posts
date: 2023-06-27T00:00:00
---

The 3.12 release of GEOS is now available [to download]({{< ref "/usage/download" >}}).

* This release includes the following new features in the C API (and of course underlying changes to the C++ code to support these features):

  - GEOSDisjointSubsetUnion, anptimized union algorithm for inputs that can be divided into subsets.
  - GEOSLineSubstring, to clip a line using a start and end fractional length.
  - GEOSEqualsIdentical, to test exact equality (with regards to order, structure, etc) of two geometries.
  - GEOSOrientPolygons, to enforce a ring orientation on all polygonal elements in the input geometry.
  - GEOSSTRtree_build, to force an immediate build of an STRtree.
  - GEOSConcaveHullByLength, to build a concave hull by removing the longest outer edges of the Delaunay Triangulation of the space between the polygons, until the specified maximm edge length is reached
  - GEOSGeomGetM, to read the M-dimensional value of a geometry.
  - GEOSVoronoiDiagram, returns the Voronoi polygons or edges of the vertices of the given geometry.
  - GEOSCoverageSimplifyVW, GEOSCoverageUnion, GEOSCoverageIsValid for working with "polygonal coverages", collections of polygons that follow coverage rules (no overlaps, exact vertex equivalence on shared edges).
  - GEOSGeom_releaseCollection, to free a collection while leaving the sub-geometries intact.
  - GEOSMinimumRotatedRectangle, a rework of the minimum rectangle code to be faster and more correct.

* GEOS has been updated to read and write "M" dimension. As with the "Z" coordinate support, not all operations can preserve the "M" dimension, but best efforts are made to retain it through calculations, input and output.

* Much work on the core code of GEOS was carried out with a maintenance grant funded by the GDAL project and [carried out by Dan Baston](https://www.mail-archive.com/gdal-dev@lists.osgeo.org/msg39018.html).

<!--more-->

The full list of changes is as follows:

- New things:
  - C++14 is now required.
  - Polygonal coverages: CoverageValidator, CoveragePolygonValidator,
    CoverageGapFinder, CoverageUnion ([JTS-900](https://github.com/locationtech/jts/issues/900), Martin Davis & Paul Ramsey)
  - Support reading and writing M values through WKB and WKT readers/writers
    ([GH-721](https://github.com/libgeos/geos/issues/721), Dan Baston)
  - Interpolate M values in overlay results ([GH-802](https://github.com/libgeos/geos/issues/802), Dan Baston)
  - CAPI: GEOSPreparedContainsXY, GEOSPreparedIntersectsXY ([GH-677](https://github.com/libgeos/geos/issues/677), Dan Baston)
  - Add CoordinateSequenceIterator ([GH-685](https://github.com/libgeos/geos/issues/685), Dan Baston)
  - Geometry clustering: DBSCAN, geometry intersection/distance, envelope
    intersection/distance ([GH-688](https://github.com/libgeos/geos/issues/688), Dan Baston)
  - CAPI: GEOSDisjointSubsetUnion ([GH-692](https://github.com/libgeos/geos/issues/692), Dan Baston)
  - CAPI: GEOSLineSubstring ([GH-706](https://github.com/libgeos/geos/issues/706), Dan Baston)
  - CAPI: GEOSEqualsIdentical ([GH-810](https://github.com/libgeos/geos/issues/810), Dan Baston)
  - CAPI: GEOSOrientPolygons ([GH-818](https://github.com/libgeos/geos/issues/818), Dan Baston)
  - CAPI: GEOSSTRtree_build ([GH-835](https://github.com/libgeos/geos/issues/835), Dan Baston)
  - CAPI: GEOSConcaveHullByLength ([GH-849](https://github.com/libgeos/geos/issues/849), Martin Davis)
  - CAPI: GEOSGeomGetM ([GH-864](https://github.com/libgeos/geos/issues/864), Mike Taves)
  - Voronoi: Add option to create diagram in order consistent with inputs ([GH-781](https://github.com/libgeos/geos/issues/781), Dan Baston)
  - Polygonal coverages: CoverageSimplifier ([JTS-911](https://github.com/locationtech/jts/issues/911), Martin Davis)
  - CAPI: GEOSCoverageIsValid, GEOSCoverageSimplifyVW ([GH-867](https://github.com/libgeos/geos/issues/867), Paul Ramsey)
  - CAPI: GEOSGeom_releaseCollection ([GH-848](https://github.com/libgeos/geos/issues/848))
  - CAPI: GEOSMinimumRotatedRectangle now uses MinimumAreaRectangle (Paul Ramsey)

- Breaking Changes
  - CoverageUnion now requires valid inputs to produce valid outputs
    and may return invalid outputs silently when fed invalid inputs.
    Use CoverageValidator first if you do not know the validity of your data.

- Fixes/Improvements:
  - WKTReader: Fix parsing of Z and M flags in WKTReader ([#676](https://trac.osgeo.org/geos/ticket/676) and [GH-669](https://github.com/libgeos/geos/issues/669), Dan Baston)
  - WKTReader: Throw exception on inconsistent geometry dimension ([#1080](https://trac.osgeo.org/geos/ticket/1080), Dan Baston)
  - WKTReader: Throw exception if WKT contains extra text after end of geometry ([#1095](https://trac.osgeo.org/geos/ticket/1095), Dan Baston)
  - GEOSIntersects: Fix crash with empty point inputs ([#1110](https://trac.osgeo.org/geos/ticket/1110), Dan Baston)
  - GEOSIntersects: Improve performance/robustness by using PreparedGeometry algorithm ([GH-775](https://github.com/libgeos/geos/issues/775), Dan Baston)
  - LineMerger: Recursively collect all components from GeometryCollections ([#401](https://trac.osgeo.org/geos/ticket/401), Dan Baston)
  - GeometryPrecisionReducer: Return correct dimensionality for empty results ([GH-684](https://github.com/libgeos/geos/issues/684), Dan Baston)
  - Improve performance of coverage union ([GH-681](https://github.com/libgeos/geos/issues/681), Dan Baston)
  - Improve performance of prepared polygon intersection ([GH-690](https://github.com/libgeos/geos/issues/690), Dan Baston)
  - Improve performance of prepared polygon distance ([GH-693](https://github.com/libgeos/geos/issues/693), Dan Baston)
  - Implement indexed calculations for prepared geometry isWithinDistance ([GH-691](https://github.com/libgeos/geos/issues/691), Dan Baston)
  - Fix LineSegment.orientationIndex(LineSegment) ([GH-699](https://github.com/libgeos/geos/issues/699), Martin Davis)
  - Fix DepthSegment comparison operation ([GH-707](https://github.com/libgeos/geos/issues/707), Martin Davis)
  - Add OverlayNG support for simple GeometryCollection inputs ([GH-716](https://github.com/libgeos/geos/issues/716), Martin Davis)
  - Fix TopologyPreservingSimplifier to produce stable results for Multi inputs ([GH-718](https://github.com/libgeos/geos/issues/718), Martin Davis)
  - Improve ConvexHull radial sort robustness ([GH-724](https://github.com/libgeos/geos/issues/724), Martin Davis)
  - Use more robust Delaunay Triangulation frame size heuristic ([GH-728](https://github.com/libgeos/geos/issues/728), Martin Davis)
  - DiscreteFrechetDistance: Fix crash with empty inputs ([GH-751](https://github.com/libgeos/geos/issues/751), Dan Baston)
  - GEOSSimplify / DouglasPeuckerSimplifier: Allow ring origin to be removed ([GH-773](https://github.com/libgeos/geos/issues/773), Dan Baston)
  - GEOSTopologyPreserveSimplify / TopologyPreservingSimplifier: Allow ring origin to be removed ([GH-784](https://github.com/libgeos/geos/issues/784), Dan Baston)
  - PreparedLineStringIntersects: Fix incorrect result with mixed-dim collection ([GH-774](https://github.com/libgeos/geos/issues/774), Dan Baston)
  - GEOSIntersection: Fix FE_INVALID exception on intersection of disjoint geometries
    ([GH-791](https://github.com/libgeos/geos/issues/791), Joris Van den Bossche & Dan Baston)
  - Fix incorrect result from Envelope::disjoint ([GH-791](https://github.com/libgeos/geos/issues/791), Dan Baston)
  - Polygonizer: Fix duplicate lines return by getInvalidRingLines ([GH-782](https://github.com/libgeos/geos/issues/782), Martin Davis & Dan Baston)
  - GEOSUnaryUnion: Fix crash on collection containing empty point ([GH-830](https://github.com/libgeos/geos/issues/830), Dan Baston)
  - GEOSBuffer: Fix crash with Inf coordinates ([GH-822](https://github.com/libgeos/geos/issues/822), Dan Baston)
  - GEOSSTRtree_iterate: Do not return removed items ([GH-833](https://github.com/libgeos/geos/issues/833), Dan Baston)
  - IndexedFacetDistance: Fix crash with Inf coordinates ([GH-821](https://github.com/libgeos/geos/issues/821), Dan Baston)
  - HausdorffDistance: Fix crash on collection containing empty point ([GH-840](https://github.com/libgeos/geos/issues/840), Dan Baston)
  - MaximumInscribedCircle: Fix infinite loop with non-finite coordinates ([GH-843](https://github.com/libgeos/geos/issues/843), Dan Baston)
  - DistanceOp: Fix crash on collection containing empty point ([GH-842](https://github.com/libgeos/geos/issues/842), Dan Baston)
  - OffsetCurve: improve behaviour and add Joined mode ([JTS-956](https://github.com/locationtech/jts/issues/956), Martin Davis)
  - GeometryPrecisionReducer: preserve input collection types ([GH-846](https://github.com/libgeos/geos/issues/846), Paul Ramsey)
  - OffsetCurve: handle zero-distance offsets ([GH-850](https://github.com/libgeos/geos/issues/850), Martin Davis)
  - Tri: add exceptions for invalid indexes ([GH-853](https://github.com/libgeos/geos/issues/853), Martin Davis)
  - LargestEmptyCircle: enhance boundary to allow any polygonal geometry ([GH-859](https://github.com/libgeos/geos/issues/859), Martin Davis)
  - Fix MaximumInscribedCircle and LargestEmptyCircle performance and memory issues ([GH-883](https://github.com/libgeos/geos/issues/883), Martin Davis)
  - GEOSHasZ: Fix handling with empty geometries ([GH-887](https://github.com/libgeos/geos/issues/887), Mike Taves)
  - OffsetCurve: fix EndCap parameter handling ([GH-899](https://github.com/libgeos/geos/issues/899), Martin Davis)
  - Reduce artifacts in single-sided Buffers: ([GH-665](https://github.com/libgeos/geos/issues/665) [#810](https://trac.osgeo.org/geos/ticket/810) and [#712](https://trac.osgeo.org/geos/ticket/712), Sandro Santilli)
  - GeoJSONReader: Fix 2D empty geometry creation ([GH-909](https://github.com/libgeos/geos/issues/909), Mike Taves)
  - GEOSClipByRect: Fix case with POINT EMPTY ([GH-913](https://github.com/libgeos/geos/issues/913), Mike Taves)
  - Support mixed GeometryCollection in overlay ops ([GH-797](https://github.com/libgeos/geos/issues/797), Paul Ramsey)

- Changes:
  - Remove Orientation.isCCW exception to simplify logic and align with JTS ([GH-878](https://github.com/libgeos/geos/issues/878), Martin Davis)
  - Change MultiPoint WKT to use parentheses in sub-members ([GH-903](https://github.com/libgeos/geos/issues/903), Mike Taves)
  - Change WKBWriter default output dimension to 4 ([GH-908](https://github.com/libgeos/geos/issues/908), Mike Taves)
  - Change WKTWriter defaults output dimension to 4 and trim to 'on' ([GH-915](https://github.com/libgeos/geos/issues/915), Mike Taves)
