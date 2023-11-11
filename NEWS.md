
## Changes in 3.12.1
2023-11-11

- Fixes:
  - Remove undefined behaviour in use of null PrecisionModel (GH-931, Jeff Walton)
  - Explicitly set endianness for some tests so that output matches expected (GH-934, Paul Ramsey)
  - Fix IncrementalDelaunayTriangulator to ensure triangulation boundary is convex (GH-953, Martin Davis)
  - Improve scale handling for PrecisionModel (GH-956, Martin Davis)
  - Fix PreparedLineStringDistance for lines within envelope and polygons (GH-959, Martin Davis)
  - Fix error in CoordinateSequence::add when disallowing repeated points (GH-963, Dan Baston)
  - Fix InteriorPointPoint to handle empty elements (GH-977, Martin Davis)
  - Skip over testing empty distances for mixed collections. (GH-979, Paul Ramsey)
  - Fix TopologyPreservingSimplifier endpoint handling to avoid self-intersections (GH-986, Martin Davis)
  - Build warnings with Visual Studio (GH-929, Jeff Mckenna, Paul Ramsey)
  - Fix CMake on Windows with Visual Studio (GH-945, Aaron Barany)

## Changes in 3.12.0
2023-06-27

- New things:
  - C++14 is now required.
  - Polygonal coverages: CoverageValidator, CoveragePolygonValidator,
    CoverageGapFinder, CoverageUnion (JTS-900, Martin Davis & Paul Ramsey)
  - Support reading and writing M values through WKB and WKT readers/writers
    (GH-721, Dan Baston)
  - Interpolate M values in overlay results (GH-802, Dan Baston)
  - CAPI: GEOSPreparedContainsXY, GEOSPreparedIntersectsXY (GH-677, Dan Baston)
  - Add CoordinateSequenceIterator (GH-685, Dan Baston)
  - Geometry clustering: DBSCAN, geometry intersection/distance, envelope
    intersection/distance (GH-688, Dan Baston)
  - CAPI: GEOSDisjointSubsetUnion (GH-692, Dan Baston)
  - CAPI: GEOSLineSubstring (GH-706, Dan Baston)
  - CAPI: GEOSEqualsIdentical (GH-810, Dan Baston)
  - CAPI: GEOSOrientPolygons (GH-818, Dan Baston)
  - CAPI: GEOSSTRtree_build (GH-835, Dan Baston)
  - CAPI: GEOSConcaveHullByLength (GH-849, Martin Davis)
  - CAPI: GEOSGeomGetM (GH-864, Mike Taves)
  - Voronoi: Add option to create diagram in order consistent with inputs (GH-781, Dan Baston)
  - Polygonal coverages: CoverageSimplifier (JTS-911, Martin Davis)
  - CAPI: GEOSCoverageIsValid, GEOSCoverageSimplifyVW (GH-867, Paul Ramsey)
  - CAPI: GEOSGeom_releaseCollection (GH-848)
  - CAPI: GEOSMinimumRotatedRectangle now uses MinimumAreaRectangle (Paul Ramsey)

- Breaking Changes
  - CoverageUnion now requires valid inputs to produce valid outputs
    and may return invalid outputs silently when fed invalid inputs.
    Use CoverageValidator first if you do not know the validity of your data.

- Fixes/Improvements:
  - WKTReader: Fix parsing of Z and M flags in WKTReader (#676 and GH-669, Dan Baston)
  - WKTReader: Throw exception on inconsistent geometry dimension (#1080, Dan Baston)
  - WKTReader: Throw exception if WKT contains extra text after end of geometry (#1095, Dan Baston)
  - GEOSIntersects: Fix crash with empty point inputs (#1110, Dan Baston)
  - GEOSIntersects: Improve performance/robustness by using PreparedGeometry algorithm (GH-775, Dan Baston)
  - LineMerger: Recursively collect all components from GeometryCollections (#401, Dan Baston)
  - GeometryPrecisionReducer: Return correct dimensionality for empty results (GH-684, Dan Baston)
  - Improve performance of coverage union (GH-681, Dan Baston)
  - Improve performance of prepared polygon intersection (GH-690, Dan Baston)
  - Improve performance of prepared polygon distance (GH-693, Dan Baston)
  - Implement indexed calculations for prepared geometry isWithinDistance (GH-691, Dan Baston)
  - Fix LineSegment.orientationIndex(LineSegment) (GH-699, Martin Davis)
  - Fix DepthSegment comparison operation (GH-707, Martin Davis)
  - Add OverlayNG support for simple GeometryCollection inputs (GH-716, Martin Davis)
  - Fix TopologyPreservingSimplifier to produce stable results for Multi inputs (GH-718, Martin Davis)
  - Improve ConvexHull radial sort robustness (GH-724, Martin Davis)
  - Use more robust Delaunay Triangulation frame size heuristic (GH-728, Martin Davis)
  - DiscreteFrechetDistance: Fix crash with empty inputs (GH-751, Dan Baston)
  - GEOSSimplify / DouglasPeuckerSimplifier: Allow ring origin to be removed (GH-773, Dan Baston)
  - GEOSTopologyPreserveSimplify / TopologyPreservingSimplifier: Allow ring origin to be removed (GH-784, Dan Baston)
  - PreparedLineStringIntersects: Fix incorrect result with mixed-dim collection (GH-774, Dan Baston)
  - GEOSIntersection: Fix FE_INVALID exception on intersection of disjoint geometries
    (GH-791, Joris Van den Bossche & Dan Baston)
  - Fix incorrect result from Envelope::disjoint (GH-791, Dan Baston)
  - Fix handling of odd cases in PolygonHoleJoiner (JTS-946, Paul Ramsey, Martin Davis)
  - Polygonizer: Fix duplicate lines return by getInvalidRingLines (GH-782, Martin Davis & Dan Baston)
  - GEOSUnaryUnion: Fix crash on collection containing empty point (GH-830, Dan Baston)
  - GEOSBuffer: Fix crash with Inf coordinates (GH-822, Dan Baston)
  - GEOSSTRtree_iterate: Do not return removed items (GH-833, Dan Baston)
  - IndexedFacetDistance: Fix crash with Inf coordinates (GH-821, Dan Baston)
  - HausdorffDistance: Fix crash on collection containing empty point (GH-840, Dan Baston)
  - MaximumInscribedCircle: Fix infinite loop with non-finite coordinates (GH-843, Dan Baston)
  - DistanceOp: Fix crash on collection containing empty point (GH-842, Dan Baston)
  - OffsetCurve: improve behaviour and add Joined mode (JTS-956, Martin Davis)
  - GeometryPrecisionReducer: preserve input collection types (GH-846, Paul Ramsey)
  - OffsetCurve: handle zero-distance offsets (GH-850, Martin Davis)
  - Tri: add exceptions for invalid indexes (GH-853, Martin Davis)
  - LargestEmptyCircle: enhance boundary to allow any polygonal geometry (GH-859, Martin Davis)
  - Fix MaximumInscribedCircle and LargestEmptyCircle performance and memory issues (GH-883, Martin Davis)
  - GEOSHasZ: Fix handling with empty geometries (GH-887, Mike Taves)
  - OffsetCurve: fix EndCap parameter handling (GH-899, Martin Davis)
  - Reduce artifacts in single-sided Buffers: (GH-665 #810 and #712, Sandro Santilli)
  - GeoJSONReader: Fix 2D empty geometry creation (GH-909, Mike Taves)
  - GEOSClipByRect: Fix case with POINT EMPTY (GH-913, Mike Taves)
  - Support mixed GeometryCollection in overlay ops (GH-797, Paul Ramsey)

- Changes:
  - Remove Orientation.isCCW exception to simplify logic and align with JTS (GH-878, Martin Davis)
  - Change MultiPoint WKT to use parentheses in sub-members (GH-903, Mike Taves)
  - Change WKBWriter default output dimension to 4 (GH-908, Mike Taves)
  - Change WKTWriter defaults output dimension to 4 and trim to 'on' (GH-915, Mike Taves)

## Changes in 3.11.0
2022-07-01

- New things:
  - OffsetCurve (GH-530, Paul Ramsey/Martin Davis)
  - ConcaveHull (GH-550, Paul Ramsey/Martin Davis)
  - ConcaveHullOfPolygons (GH-617, Paul Ramsey/Martin Davis)
  - PolygonHull (GH-603, Paul Ramsey/Martin Davis)
  - LineMerger directed option (GH-597, Sergei Sh)
  - CAPI: GEOSHilbertCode (GH-556, Brendan Ward)
  - CAPI: GEOSGeom_createRectangle (GH-558, Brendan Ward)
  - CAPI: GEOSGeom_transformXY (GH-563, Dan Baston/Brendan Ward)
  - CAPI: GEOSRemoveRepeatedPoints (GH-599, Paul Ramsey)
  - CAPI: GEOSConcaveHull (GH-550, Paul Ramsey)
  - CAPI: GEOSConcaveHullOfPolygons (GH-603, Paul Ramsey)
  - CAPI: GEOSPolygonHullSimplify (GH-603, Paul Ramsey)
  - CAPI: GEOSPolygonHullSimplifyByArea (GH-633, Paul Ramsey)
  - CAPI: GEOSLineMergeDirected (GH-597, Sergei Sh)
  - CAPI: GEOSGeom_getExtent (GH-555, Joris Van den Bossche)
  - CAPI: setFixStructure for WKB/WKT readers to automatically repair
    structural errors in the input (GH-639, Paul Ramsey)

- Fixes/Improvements:
  - Fix unaryUnion to avoid segfault with empty polygon (GH-501, Mike Taves)
  - Fix SnapRoundingNoder to use tolerance in noding; also fixes GeometryPrecisionReducer (#504, Sergei)
  - Allow direct setting of grid size (GH-513, Martin Davis)
  - Allow GEOS to be used as a CMake subproject (GH-518, Robert Coup)
  - CAPI GEOSOffsetCurve uses new OffsetCurve implementation (GH-530, Paul Ramsey)
  - Remove .inl inline files in favour of header declaration (GH-543, Paul Ramsey)
  - Add SnappingNoder seeding (Martin Davis)
  - Add OverlayNG area check heuristic (JTS-812, Paul Ramsey)
  - Fix RelateOp (and intersects predicate) for lines with intersections very near boundary (GH-570, Martin Davis)
  - Fix IsValidOp to handle repeated node points (JTS-845, Martin Davis)
  - Fix IsSimpleOp to handle closed LineStrings with repeated endpoints (JTS-851, Martin Davis)
  - Fix LengthIndexedLine (via LengthLocationMap fix) (JTS-859, Martin Davis)
  - Fix PolygonHoleJoiner (JTS-862, Martin Davis)
  - Improve `test_geos_unit` application error checking and reporting
  - Fix MinimumDiameter getMinimumRectangle for flat input (JTS-875, Martin Davis)
  - Fix BufferOp inverted ring check (JTS-878, Martin Davis)
  - Fix OverlayNG geomunion to avoid lines in result (Martin Davis)


## Changes in 3.10.0
2021-10-20

- New things:

  - CAPI: GEOSDistanceWithin, GEOSPreparedDistanceWithin
          (#1124, Sandro Santilli)
  - Output WKT using positional precision with the ryu
    library (#868, Paul Ramsey)
  - geosop CLI for GEOS (Martin Davis)
  - Full doxygen of the C-API (Paul Ramsey)
  - GeometryFixer class for validity enforcement (Martin Davis, Paul Ramsey)
  - PolygonTriangulator, ConstrainedDelaunayTriangulator (Martin Davis, Paul Ramsey)
  - GeoJSON reader/writer implementation (Jared Erickson, WhuAegeanSea)
  - CAPI: GEOSDensify (Brendan Ward)
  - CAPI: GEOSCoordSeq_copyFromArrays, GEOSCoordSeq_copyFromBuffer,
          GEOSCoordSeq_copyToArrays, GEOSCoordSeq_copyToBuffer (Daniel Baston)
  - CAPI: GEOSMakeValidWithParams new validity enforcement approach from
          https://github.com/locationtech/jts/pull/704, uses GeometryFixer
          (Paul Ramsey, Martin Davis)
  - CAPI: GEOSWKBWriter_getFlavor, GEOSWKBWriter_setFlavor support
          outputting ISO or Extended WKB flavors (#466, Paul Ramsey)
  - CAPI: GEOSConstrainedDelaunayTriangulation, builds a constrained
          triangulation of an input Polygon or MultiPolygon,
          returning a GeometryCollection(Polygon) of the triangles.
  - CAPI: GEOSGeoJSONReader_create, GEOSGeoJSONReader_destroy,
          GEOSGeoJSONReader_readGeometry, GEOSGeoJSONWriter_create,
          GEOSGeoJSONWriter_destroy, GEOSGeoJSONWriter_writeGeometry
          (Casper van der Wel)

- Fixes/Improvements:
  - Improved IsValidOp (Paul Ramsey, Martin Davis)
  - Improved IsSimpleOp (Paul Ramsey, Martin Davis)
  - Preserve ordering of lines in overlay results (Martin Davis)
  - Check for invalid geometry before fixing polygonal result in Densifier
    and DPSimplifier (Martin Davis)
  - Fix overlay handling of flat interior lines (JTS-685, Martin Davis)
  - Fix IsValidOp to correctly report invalid nested MultiPolygons (#1112, Martin Davis)
  - Fix BufferOp to avoid artifacts in certain polygon buffers (#1101, Martin Davis)
  - Fix IsValidOp to correctly report certain kinds of invalid LinearRings (Martin Davis)
  - Improve STRtree performance through TemplateSTRtree implementation (Daniel Baston)
  - Fix DiscreteFrechetDistance to use initial points of input lines (#1128, Martin Davis)

- Changes:
  - Autoconf build system is dropped in favour of CMake
    See README.md and INSTALL.md for examples of usage
  - Libtool is no longer used for in favour of CMake
    Therefor libgeos.la is no longer built/installed
  - #1094, #1090: Drop inlines.cpp to address duplicate symbols on many platforms
    (Regina Obe)
  - GH475: Do not return NaN from GEOSProjectNormalized_r (Paul Ramsey)
  - GH442: BufferOp now returns unique_ptr<Geometry> result (Paul Ramsey)

## Changes in 3.9.0
2020-12-09

- New things:
  - OverlayNG engine from JTS, now the default (Martin Davis, Paul Ramsey)
  - MaximumInscribedCircle and LargestEmptyCircle (JTS-530, Paul Ramsey)
  - CAPI: Fixed precision overlay operations (Sandro Santilli)
  - CAPI: GEOSPreparedNearestPoints (#1007, Sandro Santilli)
  - CAPI: GEOSPreparedDistance (#1066, Sandro Santilli)
  - CAPI: GEOSGeom_setPrecision uses OverlayNG (Paul Ramsey)
  - SimpleSTRTree spatial index implementation (Paul Ramsey)
  - Add support for pkg-config for GEOS C API (#1073, Mike Taves)

- Fixes/Improvements:
  - Stack allocate segments in OverlapUnion (Paul Ramsey)
  - Improve performance of GEOSisValid (Dan Baston)
  - Update geos-config tool for consistency
    and escape paths (https://git.osgeo.org/gitea/geos/geos/pulls/99)
    changes mostly affect CMake MSVC builds (#1015, Mike Taves)
  - Testing on Rasberry Pi 32-bit (berrie) (#1017, Bruce Rindahl, Regina Obe)
  - Replace ttmath with JTS DD double-double implementation (Paul Ramsey)
  - Fix bug in DistanceOp for geometries with empty components (#1026, Paul Ramsey)
  - Remove undefined behaviour in CAPI (#1021, Greg Troxel)
  - Fix buffering issue (#1022, JTS-525, Paul Ramsey)
  - MinimumBoundingCircle.getMaximumDiameter fix (JTS-533, Paul Ramsey)
  - Output POINT EMPTY in WKB as POINT (NaN NaN) (#1005, Paul Ramsey)

- Changes:
  - Drop SWIG bindings, including for Ruby and Python (#1076, Mike Taves)

## Changes in 3.8.0
2019-10-10

- New things:
  - CAPI: GEOSBuildArea (#952, Even Rouault)
  - CAPI: GEOSMakeValid (#952, Even Rouault)
  - CAPI: GEOSPolygonize_valid (#727, Dan Baston)
  - CAPI: GEOSCoverageUnion (Dan Baston)
  - CAPI: GEOSCoordSeq_setXY, GEOSCoordSeq_setXYZ,
          GEOSCoordSeq_getXY, GEOSCoordSeq_getXYZ (Dan Baston)
  - CAPI: GEOSMinimumBoundingCircle (#735)
  - CAPI: GEOSGeom_createPointFromXY (Dan Baston)

- Improvements:
  - Improve overall performance by reducing of heap allocations (Dan Baston)
  - Improve performance and robustness of GEOSPointOnSurface (Martin Davis)
  - Improve performance of GEOSPolygonize for cases with many potential
    holes (#748, Dan Baston)
  - Support extended precision calculations (ttmath) and port JTS
    improvements related to extended precision (Paul Ramsey, Mateusz Loskot)
  - Improve performance of GEOSPolygonize for cases with many or complex
    shells (Dan Baston, Martin Davis)
  - Improve performance of Delaunay triangulations / Voronoi Diagrams
    (Dan Baston)
  - Improve performance of prepared geometry operations (Dan Baston)
  - Improve robustness of Delaunay triangulations (Paul Ramsey, Martin Davis)
  - Improve performance of unary union for lines (Dan Baston)
  - Improve general predicate, overlay, and buffer performance (Dan Baston, Paul Ramsey)
  - Improve cascaded union performance (Paul Ramsey, Martin Davis)
  - Allocate default GeometryFactory singleton on the stack (Sandro Mani)
  - Harmonize XML tests with JTS and harmonize cmake/autoconf
    test running (Paul Ramsey)
  - CMake modernization (Mateusz Loskot, Paul Ramsey, Dan Baston)
  - Return unique_ptr from most methods that produce a new geometry (Dan Baston)

- Changes:
  - Constructive geometry functions in CAPI now preserve SRID
    of input arguments in output (#896)


## Changes in 3.7.2
2019-05-02

- Bug fixes / improvements
  - Envelope constructor using strtod (#875 Paul Ramsey)
  - Failure in CMake for OSX Framework (#936 Paul Ramsey)
  - Polygon build failure in Overlay difference (#789 Paul Ramsey)
  - Invalid union result from valid polygon inputs (#838)


## Changes in 3.7.1
2018-11-29

- Bug fixes / improvements
  - Fix crash in GEOSCoordSeq_isCCW with empty coordseq
    (#927, Sergey Fedoseev)
  - Fix crash in GEOSInterpolate with empty LineString
    (#926, Sergey Fedoseev)
  - Fix crash in GEOSUnaryUnion with empty LineString
    (#928, Sergey Fedoseev)
  - Fix memory leak in SIRtree::insert (#919, Dan Baston)
  - Reduce required autoconf to 2.63
    (#56, John Harvey)
  - Fix incorrect return values on error from GEOSLength
    GEOSisValidDetail (#941, Dan Baston)


## Changes in 3.7.0
2018-09-10

- New things:
  - CAPI: GEOSDistanceIndexed (#795, Dan Baston)
  - CAPI: GEOSCoordSeq_isCCW (#870, Dan Baston)
  - CAPI: GEOSGeom_getXMin, GEOSGeom_getXMax,
          GEOSGeom_getYMin, GEOSGeom_getYMax (#871, Dan Baston)
  - CAPI: GEOSFrechetDistance (#797, Shinichi SUGIYAMA)
  - CAPI: GEOSReverse (#872, Dan Baston)
  - CAPI: GEOSSegmentIntersection (#873, Dan Baston)
  - CAPI: GEOSGeomGetZ (#581, J Smith)

- Improvements
  - Interruptible snap operation (Paul Ramsey)
  - Numerous packaging, doc, and build changes
    (Debian group: Bas Couwenberg, Francesco Paolo Lovergine)

- C++ API changes:
  - Require defining USE_UNSTABLE_GEOS_CPP_API for use without
    warnings.
  - Make C++11 required (Mateusz Loskot)
  - Use C++11 unique_ptr, nullptr, and override constructs
    (Mateusz Loskot)
  - C++11 standard delete on noncopyable
    (#851, Vicky Vergara)
  - Fix CommonBits::getBit to correctly handle i >= 32
    (#834, Kurt Schwehr)

## Changes in 3.7.0rc1
2018-08-19
Fixes / enhancements since 3.7.0beta2
  - Avoid segfault when querying empty tree (#730, framm)
  - Collection with Empty components crashes overlay (#782, Dan Baston)
  - Allow static library with C API for CMake builds (#878, Dakota Hawkins)

## Changes in 3.7.0beta2
2018-08-06
Fixes since 3.7.0beta1
  - Fix infinite loop in GEOSClipByRect (#865, Dan Baston)
  - Make GEOSException inherit from std::runtime_error
    to address clang warnings (Dan Baston)
  - Add missing CMake files to tarball (#895, Regina Obe)


## Changes in 3.7.0beta1
2018-06-25
See 3.7.0 latest notes

## Changes in 3.7.0alpha
2018-06-14
See 3.7.0 notes

## Changes in 3.6.1
2016-12-24

- Bug fixes / improvements
  - Fix GEOSSTRtree_nearest_r signature and add implementation (#796)
  - Fix --static-clibs and --static-cclibs returns from geos-config
  - Fix WKB representation of empty polygon (#680).
  - Fix empty GEOSSimplify return on inner ring collapse (#741)

## Changes in 3.6.0
2016-10-25

- Important / Breaking Changes:
  - The PHP binding moved to its own repository:
    http://git.osgeo.org/gogs/geos/php-geos (#765)
- New things:
  - CAPI: GEOSGeom_{get,set}UserData (Rashad Kanavath)
  - CAPI: GEOSGeom_{set,get}Precision (#713)
  - CAPI: GEOSMinimumRotatedRectangle and GEOSMinimumWidth
    (#729, Nyall Dawson)
  - CAPI: GEOSSTRtree_nearest (#768, Dan Baston)
  - CAPI: GEOSMinimumClearance and GEOSMinimumClearanceLine
    (#776, Dan Baston)
- C++ API changes:
  - Automatic memory management for GeometryFactory objects


## Changes in 3.5.0
2015-08-16

- New things:
  - Voronoi API (#627)
  - PHP: Geometry->normalize method
  - GEOS_USE_ONLY_R_API macro support (#695)
  - PHP: WKBReader->read() & WKBWriter::write() methods (Benjamin Morel)
  - GEOSClipByRect (#699, Mika Heiskanen, Sandro Santilli)
  - CAPI: thread-safe message handling API (#663, Pepijn Van Eeckhoudt)
- Improvements:
  - Speed-up intersection and difference between geometries
    with small bounding box overlap.
  - CAPI: add MULTILINESTRING support for GEOSisClosed (Benjamin Morel)
- C++ API changes:
  - Added no-parameter CoordinateSequenceFactory::create method (Sandro Mani)

## Changes in 3.4.2
2013-08-25

- Bug fixes / improvements
  - Use a double for PrecisionModel scale, avoiding overflows
    should fix 32-bit regression failures (#652)
  - isnan workaround OS detection missing NetBSD, DragonFly, Sun nuance (#650)
  - Do not distribute platform.h and version.h, but install both (#601)
  - Non-standard ChangeLog file in 3.4.0 and 3.4.1 releases (#654)
  - new travis bot (#657)
  - accept multiple Nan representations (#656)


## Changes in 3.4.1
2013-08-17

- Bug fixes / improvements
  - Assertion failure snapping line to points of rectangle
    smaller than tolerance (#649)
  - Can't build using cmake with tar ball (#644)

## Changes in 3.4.0
2013-08-11

- New things:
  - Delaunay Triangulation API (#487, #565, #570, #567)
  - Interruptibility API (C and C++)
  - CAPI: GEOSNode (#496) - PHP: Geometry->node
  - GeometryPrecisionReducer class (#496, #526)
  - BufferInputLineSimplifier header exposed (#548)
  - New Centroid class supporting mixed geometry components (#612)
  - io::Writer::reserve() method
  - CAPI: GEOSNearestPoints
  - Add --cclibs, --static-clibs and --static-cclibs to geos-config (#497)
  - Early bail out of overlay exception if input is invalid

- C++ API changes:
  - New noding::GeometryNoder class
  - Added BufferOp::setSingleSided
  - Signature of most functions taking a Label changed to take it
    by reference rather than pointer.
  - Signature of most functions taking an IntersectionMatrix changed
    to take it by reference rather than pointer.
  - GraphComponent::label is now a Label value (from a pointer)
  - NodedSegmentString takes ownership of CoordinateSenuence now
  - io::Writer's toString() returns by const ref, write() takes a const ref
  - Unify prototypes of WKTReader and WKBReader constructor (#310)
  - GeometryCollection::computeEnvelopInternal and
    GeometryCollection::compareToSameClass are marked virtual (#478)

- Bug fixes / improvements
  - A point interpolated from a line does not always intersect
    the same line (#323)
  - Port ConvexHull robustness fix from JTS-1.13 (#457)
  - Improve Overlay robustness by reducing input precision on topology
    exception and by refusing to accept unnoded output (#459)
  - Improve Buffer robustness by reducing input precision on topology
    exception (#605)
  - Mismatch segment sides in OffsetCurveBuilder (#633 )
  - Fixed Linear Referencing API to handle MultiLineStrings consistently
    by always using the lowest possible index value, and by trimming
    zero-length components from results (#323)
  - Fixed CMake configuration to set correct SOVERSION (current - age)
  - Fix EMPTY return from single-point lines and zero-length polygons (#612)
  - CMakeLists.txt, tools/geos_svn_revision_cmake.h.in: Add
	  geos_svn_revision.h generator to CMake config (#643)
	- Makefile.vc 'clean' step leaks obj files (#607)

## Changes in 3.3.9
2013-09-04

- Bug fixes / improvements
    - Fix OffsetCurve op in presence of duplicated vertices (#602)
    - Fix LineSegmentVisitor copy ctor (#636)
    - Fix area boundary return from GEOSPointOnSurface (#623)
    - Speedup GEOSWKBReader_read (#621)
    - Fix RobustLineIntersector handling of invalid intersection points (#622)
    - Reduce likelyhood of invalid output from snap operation (#629, #501)
    - Reduce memory fragmentation of prepared Polygon/Point intersection op
    - Fix mingw64 compile (#630)
    - Fix bug in HotPixel constructor (#635)
    - Fix install location of linearref headers (#624)
    - Fix multi-geometry constructor to drop SRID from components (#583)

## Changes in 3.3.8
2013-02-28

- Bug fixes / improvements
    - IsValidOp: throw proper error on nested shells (#608)
    - Fix header guards (#617, #618, #619)
    - WKTWriter::appendCoordinate optimisation
    - Fix centroid computation for collections with empty components (#582)

## Changes in 3.3.7
2013-01-22

- Bug fixes / improvements
    - Fix abort in RightmostEdgeFinder (#605)
    - Do not force precision reduction below 6 significant digits
      while trying to obtain a valid Buffer output (#605)
    - Fix GEOSPointOnSurface with zero-length linestring (#609)
    - Fix EMPTY return from zero-area polygon (#613)
    - Segfault from symdifference (#615)

## Changes in 3.3.6
2012-11-15 -- that's Post-GIS day !

- Bug fixes / improvements
    - Add support for testing with phpunit 3.6 (not loosing support for 3.4)
    - Segfault from intersection (#586, #598, #599)

## Changes in 3.3.5
2012-06-25

- Bug fixes / improvements
    - Correctly increment CAPI lib version from 3.3.3 (#558)
    - Port robustness fix to CentroidArea (#559)
    - Always return POINT from GEOSGetCentroid, even for EMPTY (#560)
    - Always return POINT from GEOSPointOnSurface, even for EMPTY (#561)

## Changes in 3.3.4
2012-05-31

- Bug fixes / improvements
    - Do not abort on NaN overlay input (#530)
    - Reduce CommonBitsRemover harmful effects during overlay op (#527)
    - Better cross-compiler support (#534)
    - Enable overlay ops short-circuits (#542)
    - Envelope-based short-circuit for symDifference (#543)
    - Fix support for PHP 5.4 (#513)
    - Fix TopologyPreservingSimplifier invalid output on closed line (#508)
    - Reduce calls to ptNotInList, greatly speeding up Polygonizer (#545)

## Changes in 3.3.3
2012-04-01

- Bug fixes / improvements
    - Fix simplification of collections with empty items (#519)
    - Fix MSVC compilation of ambiguous log() call (#506)
    - Fix CMake issues with std:: namespace detection (#493)

## Changes in 3.3.2
2012-01-05

- Bug fixes / improvements
    - Fix CMAKE_CXX_FLAGS overriding -std=gnu++0x (#489)
    - Missing versions update in CMake configuration (#490)
    - Fix noding of self-intersecting lines through UnaryUnion (#482)
    - Fix handling of collapsed edges skipping in BufferOp (#494)
    - Print up to 18 digits of precision for TopologyException points
    - Fix noding with reduced precision in Buffer operation (#473)
    - Fix HotPixel original point invalidation (#498)
    - Fix CascadedPolygonUnion to discard non-polygonal components (#499)
    - Improve buffer robustness by reverting to non-snaprounding noder (#495)
    - Fix C++11 build by avoiding std::pair<auto_ptr> (#491)
    - Add --clibs to geos-config and GEOS_C_LIBS to geos.m4 (#497)
    - Apply shoelace formula for area calculation (#485)
    - Fix default initialization issue for clang (#500)
    - Improve overlay robustness by fixing areal validity on snapping (#488)

## Changes in 3.3.1
2011-09-27

- Bug fixes / improvements
  - Fix memory leak on invalid geometry in InteriorPointArea (#475)
  - ValidOp abort in presence of 2 touching holes forming an island (#449)
  - Enable prepared intersects operation for points
  - Fortify suspicious code found by static analysis tools
  - Fix for SOLARIS build (#461)
  - Fix EMPTY result from GEOSOffsetCurve with distance 0 (#454)
  - Fix Geometry::clone to copy SRID (#464)
  - Fix for clang builds (#463)
  - Fix out-of-place builds for python binding (#332) and regress testing
  - Fix OS X framework cmake build (#385)

## Changes in 3.3.0
2011-05-30

- New things:
  - CAPI: GEOSBufferWithParams (allows single sided buffers)
  - CAPI: GEOSOffsetCurve deprecates GEOSSingleSidedBuffer
  - CAPI: GEOSUnaryUnion deprecates GEOSCascadedUnion
  - CAPI: GEOSisValidDetail: tell state, reason & location apart. allows
          passing flags.
  - CAPI: GEOSContext_setNoticeHandler_r, GEOSContext_setErrorHandler_r
  - CAPI: GEOSGeom_createEmptyPoint, GEOSGeom_createEmptyLineString
          GEOSGeom_createEmptyPolygon, GEOSGeom_createEmptyCollection
  - CAPI: GEOSGeom_extractUniquePoints
  - CAPI: GEOSGetGeometryN support for single geometries
  - CAPI: GEOSPolygonize_full to return all informations computed by
          the polygonizer
  - CAPI: GEOSOrientationIndex
  - CAPI: GEOSSharedPaths to find shared paths and their orientation
  - CAPI: GEOSSnap
  - CAPI: GEOSRelatePatternMatch
  - CAPI: GEOSCovers, GEOSCoveredBy (#396)
  - CAPI: GEOSRelateBoundaryNodeRule (#399)
  - CAPI: GEOSGeom_getCoordinateDimension() (#311)
  - CAPI: GEOSWKTWriter_setOutputDimension, GEOSWKTWriter_setOld3D (#292)
  -  PHP: new PHP5 bindings based on CAPI
- Semantic C++ API changes:
  - Geometry inheritance chain changed to introduce Puntal, Lineal
    and Polygonal classes (virtual inheritance introduced)
  - Polygonizer::getInvalidRingLines retains ownership of vector elements
  - Geometry::isWithinDistance method is now const
  - Polygonizer::getCutEdges returns by const ref
  - Polygonizer::getDangles returns by const ref
  - Empty LinearRings are closed by definition
- Bug fixes / improvements
  - Fixed Geometry.distance() and DistanceOp to return 0.0 for empty inputs
  - Invalid compound geometries reported as valid (#333)
  - Return up to 15 digits of precision from GEOSisValidReason_t (#329)
  - CAPI: do not leak contexts when using the non-reentrant interface
  - Fix duplicated dangles returned by Polygonizer
  - Fix SnapIfNeededOverlayOp to throw the originating exception
  - Fixed LineMerger to skip lines with only a single unique coordinate
  - Fix NodedSegmentString to handle zero-length line segments correctly
    (via safeOctant)
  - Fixed buffer OffsetCurveSetBuilder to handle "flat" rings correctly
  - Added illegal state check in LineSegment::pointAlongOffset()
  - Improved performance of RectangleIntersects by always using
    segment-scanning and refining SegmentIntersectionTester
  - Reduce memory use in PreparedGeometry predicates (#342)
  - Fix infinite loop in RobustDeterminant with nan/inf input (#357)

## Changes in 3.2.0
2009-12-14

- Add Single-sided buffer operation
- JTS-1.10 sync ...
  - Drastically improved Buffer speed (20x for a case in testsuite)
  - Improved EdgeList duplicate edge finding
  - Added algorithm::distance package
  - Added algorithm::Angle class
  - Added algorithm::BoundaryNodeRule class
  - IsSimpleOp can now return non-simple location coordinate
  - DistanceOp can now check for 'within distance' predicate
    (earlier exit)
  - MultiPolygon::getBoundary always return MultiLineString, also
    when the result is the EMPTY geometry.
- Various bug and leak fixes, optimizations
- Replace MarkupSTL with tinyXML

## Changes in 3.1.0

- PreparedGeometry operations for very fast predicate testing.
  - Intersects()
  - Covers()
  - CoveredBy()
  - ContainsProperly()
- Easier builds under MSVC and OpenSolaris
- Thread-safe CAPI option
- IsValidReason added to CAPI
- GEOSPolygonizer_getCutEdges added to CAPI
- CascadedUnion operation for fast unions of geometry sets
- Numerous bug fixes.
  http://trac.osgeo.org/geos/query?status=closed&milestone=3.1.0&order=priority

## Changes in 3.0.0

   These are mostly ABI breaking changes.
   In few cases the API also changed, but the most external one
   (the documented one) should be unchanged.

- New things:
	- Added geom::BinaryOp class performing a binary operation
	  using different heuristics to reduce probability of robustness
	  issues. Both C-API and XMLTester now use this class for
	  binary operations.
	- Added covers() and coveredBy() predicates to Geometry class
	- Added overlay::overlayOp() adapter class
	- Added GEOSSimplify() and GEOSTopologyPreserveSimplify()
	  to the C API
	- Added closed ring checks in IsValidOp
	- Multi-input support in XMLTester
	- HEXWKB I/O
	- Envelope(string) ctor
	- Ruby interface
	- New ShortCircuitedGeometryVisitor class
	- New operation/predicate package
	- Added CGAlgorithms::isPointInRing() version working with
	  Coordinate::ConstVect type (faster!)
	- Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence
	  class.
	- Moved GetNumGeometries() and GetGeometryN() interfaces
	  from GeometryCollection to Geometry class.
	- New planarSubgraph class
	- New ConnectedSubgraphFinder class.
	- New LineSequencer class
	- New WKTWriter::toLineString and ::toPoint convenience methods
	- New IsValidOp::setSelfTouchingRingFormingHoleValid method
	- New WKTWriter::toLineString and ::toPoint convenience methods
	- New IsValidOp::setSelfTouchingRingFormingHoleValid method
	- New Envelope::centre()
	- New Envelope::intersection(Envelope)
	- New Envelope::expandBy(distance, [ydistance])
	- New LineString::reverse()
	- New MultiLineString::reverse()
	- New Geometry::buffer(distance, quadSeg, endCapStyle)
	- New SnapRounding code
	- New size() and operator[] interfaces to CoordinateSequence
	- New ScaledNoder class
	- New unit tests (make check rule)

- Optimizations:
	- WKT parser speedup
	- Function inlining
	- Coordinate copies reduction
  	- Heap allocations reduction
	- More classes made final
	- Better use of standard containers
	- Use of singletons when appropriate
	- Removed many function calls in loops' end conditions
	- Improved XMLTester output and user interface
	- Improved memory use in geos::geom::util::PolygonExtractor
	- Ported JTS-1.7 version of ConvexHull with big attention to
	  memory usage optimizations.
	- Changed CoordinateArrayFilter to reduce memory copies
	- Changed UniqueCoordinateArrayFilter to reduce memory copies
	- Added rectangle-based optimizations of intersects() and
	  contains() ops
	- Inlined all planarGraphComponent class
	- More iterators returning methods and inlining in planargraph.
	- Obsoleted toInternalGeometry/fromInternalGeometry
	- Improved buffering speed and robustness by using Snap Rounding

- Semantic changes

	- SegmentString: getCoordinates() doesn't return a clone
	  anymore, getCoordinatesRO() obsoleted.
	- JTS packages mapped to geos:: sub-namespaces
	- Geometry::getInteriorPoint() returns NULL if called
	  against an EMPTY geom
	- LineString::get{Start,End}Point return NULL for
	  EMPTY geoms
	- GEOSException is now derived by std::runtim_exception
	  and thrown by const reference.
	- Geometry constructors made protected, to force use
	  of a GeometryFactory.

- Correctness:
	- More const-correct signatures
	- Stronger methods typing (removed some void * args).
	- Changed index-related funx signatures to use size_t
	  rather then int
	- More const-correctness in Buffer "package"
	- Bugfix in LineString::getCoordinate() failing to return
	  NULL from getCoordinat() when empty.
	- Use unsigned int for indexes and sizes.

- Layout changes:
	- Namespaces mapping JTS packages
	- Renamed classes after JTS names (namespaces use made this possible
	  w/out name clashes)
	- Splitted headers, for build speedup and possible API reduction.
	- Moved source/bigtest and source/test to tests/bigtest
	  and test/xmltester
	- Moved C-API in it's own top-level dir capi/
	- Reworked automake scripts to produce a static lib for each subdir
	  and then link all subsystem's libs togheter
	- Renamed DefaultCoordinateSequence to CoordinateArraySequence.
	- Renamed OverlayOp opcodes by prepending the 'op' prefix, and
	  given the enum a name (OpCode) for type-safety.

- Bug fixes:
	- Fixed bug causing redundant linestrings to be returned in the
	  result of overlaying polygons containing touching holes (#13)
	- Fixed integer conversion bug
	- Fixed PointLocator handling of LinearRings
	- Added missing ::clone() methods for Multi* geoms

- (Partial) Detailed list of changes:
	- Changed SegmentNode to contain a *real* Coordinate (not a pointer)
  	  to reduce construction costs.
	- Changed geomgraph nodeMap to use Coordinate pointers as keys
	- Envelope destructor made non-virtual to give compiler more static
	  binding options.
	- Changed BufferSubgraph::computeDepths to use a set instead of a
	  vector for checking visited Edges.
	- Made LineIntersector a concrete type
	- Node::isIncidentEdgeInResult() method made virtual
	- Const-correct signatures in LineMerger package
	- Changed operation/valid/*NestedRingTester classes interface
	  to use Coordinate pointers instead of copies.
	- Changed EdgeIntersectionList to use a set instead of a vector
	- Changed DepthSegment to store a real Coordinate rather then a pointer.
	- Changed SubgraphDepthLocater to store real containers rather then
	  pointers.
	- Changed BufferSubgraph to store a real RightmostEdgeFinder and real
 	  containers rather then pointers.
	- CoordinateSequence API changes:
		- point index and size related functions
	  	  use unsigned int rather then int
	- Changed EdgeEndStar to maintain a single container for EdgeEnds
	- Changed PlanarGraph::addEdges to take a const vector by ref
	  rathern then a non-const vector by pointer
	- Changed EdgeList::addAll to take a const vector by ref
	  rather then a non-const vector by pointer
	- Added apply_rw(CoordinateFilter *) and apply_ro(CoordinateFilter *)
	  const to CoordinateSequence
	- LineBuilder::lineEdgesList made a real vector, rather then pointer
	  (private member)
	- SegmentString::eiList made a real SegmentNodeList, rather then
	  a pointer (private member)
	- Removed coordinate copies in ElevationMatrix::elevate
	- Changed CoordinateFilter interface to have a const method
	  for filter_rw, updated interfaces using this to take
	  const CoordinateFilter (apply_rw).


## Changes in 2.2.4

	- Added version.in.vc to distribution

## Changes in 2.2.1

- Support for MingW builds
- Bugfix in Polygonizer chocking on invalid LineString inputs
- CAPI: small leak removed in GEOSHasZ()

## Changes in 2.2.0

- Performance improvement in OverlayOp::insertUniqueEdge()
- CoordinateSequence copy removal in EdgeRing
- Minor memory allocation improvements
- Higher dimensions interface for CoordinateSequence
- Added getCoordinatesRO for Point class
- NEW WKB IO
- NEW Simplified and stabler C API

## Changes in 2.1.4

- Severe BUGFIX in BufferSubgraphGT and other functions used
  as StrictWeakOrdering predicates for sort()

## Changes in 2.1.3

- win32/mingw build support
- Segfault fix in LinearRing and LineString constructors
- Segfault fix in Polygonizer
- XMLTester installed by default
- XMLTester code cleanup
- Fixed handling of collection input in GeometryFactory::buildGeometry
- Added shortcircuit test for Union operation
- Reduced useless Coordinate copies in CGAlgorithms::isPointInRing()
- Performance improvements in CGAlgorithms::isOnLine()
- Other minor performance improvements
- New Node::isIncidentEdgeInResult() method
- OverlayOp's PointBuilder performance improvement by reduction
  of LineIntersector calls.
- Optimizations in Buffer operation
- Sever BUGFIX in DepthSegmentLT as suggested by Graeme Hiebert

## Changes in 2.1.2

- Segfault fix in Point::isEmpty
- Mem Leak fix in OffsetCurveBuilder::getRingCurve
- Bugfix in LineSegment::reverse
- Added multipolygon buffering test in source/test/testLeaksBig
- Ported JTS robustness patch for RobustLineIntersector
- Removed useless Coordinate copies in OverlayOp::mergeZ()
- Avoided throws by IsValid on invalid input
- Stricter C++ syntax (math.h=>cmath, ieeefp.h in "C" block, ostringstream
  instead of sprintf)
- Better support for older compilers (Polygonizer::LineStringAdder friendship)
- Removed useless Coordinate copies in CGAlgorithms::isOnLine()
- Added support for polygonize and parametrized buffer tests in XMLTester
- Fixed support for --includedir and --libdir
- Fixed Z interpolation in LineIntersector
- Handled NULL results from getCentroid() in XMLTester
- Segfault fix in (EMPTY)Geometry::getCentroid()
- Made polygon::getBoundary() always OGC-valid (no LinearRings)
- Input checking and promoting in GeometryFactory::createMultiLineString()
- Segfault fix in GeometryEditor::editPolygon()


## Changes in 2.1.1

- Fixed uninitialized Coordinate in TopologyException
- Added install of version.h, platform.h and timeval.h
- Memleak fix in PolygonizeGraph
- Memleak fix in OverlayOp
- Compiler warnings removal
- Cleaner w32 build
- Z interpolation in overlay operations
- Debian package build scripts


## Changes in 2.1.0

- Added Polygonizer and LineMerger classes.
- python wrapper examples
- General cleanup / warnings removal
- cleaner win32 / older copilers builds
- Reduced heap allocations
- debian package builder scripts
- reduction of standard C lib headers dependency
- Z support in overlay operations.


## Changes in 2.0.0

- CoordinateList renamed to CoordinateSequence, BasicCoordinateList
  renamed to DefaultCoordinateSequence to reflect JTS changes.
  DefaultCoordinateSequenceFactory and CoordinateSequenceFactory
  got same interface as JTS.
- Added geos/version.h defining versioning infos
- Added geos.h for quick inclusion. It will include geos/geom.h,
  new geos/version.h, geos/util.h geos/io.h and geos/unload.h
  (geometry input/output, exceptions, operations).
- Added a geos::version() function showing GEOS and equivalent
  JTS versions as strings.
- All geometry constructors take ownership of given arguments.
  GeometryFactory provides pass-by-reference geometry creators
  to take care of a deep-copy.
- GeometryFactory::createMultiPoint(const CoordinateList *)
  has been renamed to
  GeometryFactory::createMultiPoint(const CoordinateList &)
  to reflect copy semantic
- GeometryFactory: EMPTY geometry creation do now have their
  own constructors taking no arguments.
- Geometry constructors taking PrecisionModel and SRID have
  been dropped. You have to use GeometryFactory instead.
- WKTWriter default constructor has been dropped. You need
  to initialize it with an explicit GeometryFactory
