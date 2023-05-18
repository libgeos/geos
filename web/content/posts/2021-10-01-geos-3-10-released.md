---
title: "Version 3.10.0"
type: posts
date: 2021-10-21T00:00:00
---

The 3.10 release of GEOS is now available [to download]({{< ref "/usage/download" >}}).

* This release includes the following new features in the C API (and of course underlying changes to the C++ code to support these features):

  * CAPI additions for testing whether geometries are within a distance of each other, `GEOSDistanceWithin` and `GEOSPreparedDistanceWithin`
  * CAPI addition for adding extra vertices to a geometry, `GEOSDensify`
  * CAPI additions for high-performance construction/reading of coordinate sequences from/to memory buffers, `GEOSCoordSeq_copyFromArrays`, `GEOSCoordSeq_copyFromBuffer`, `GEOSCoordSeq_copyToArrays`, and `GEOSCoordSeq_copyToBuffer`
  * CAPI addition for new validity enforement algorithm, `GEOSMakeValidWithParams`
  * CAPI addition for ISO WKB output, `GEOSWKBWriter_getFlavor` and `GEOSWKBWriter_setFlavor`
  * CAPI addition to create a constrained delaunay of polygonal input, `GEOSConstrainedDelaunayTriangulation`

<!--more-->

* There is a new utility for running arbitrary GEOS commands against input files, `geosop`. See the [user guide](https://github.com/libgeos/geos/blob/e286d4afd4826fd1886a4ac18e32a3c026e11394/util/geosop/README.md) for examples.

* The C API is now fully documented and available as a [reference document](http://libgeos.org/doxygen/geos__c_8h.html).

* The IsValidOp and IsSimpleOp have been re-written for higher performance in general and faster response for "simple" cases of invalidity.

* The STRtree has been replaced with a templated version that is even faster than before. This has improved algorithm performance across the board.

* There have been numerous other bug fixes and performance tweaks.


