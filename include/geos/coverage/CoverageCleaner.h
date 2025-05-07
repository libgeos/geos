/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

// #include <geos/noding/BasicSegmentString.h>


// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Geometry;
class GeometryFactory;
class LineString;
class LinearRing;
class Polygon;
}
}

namespace geos {         // geos.
namespace coverage { // geos.coverage


/**
 * Cleans the linework of a set of polygonal geometries to form a valid polygonal coverage.
 * The input is an array of valid Polygon or MultiPolygon geometries
 * which may contain topological errors such as overlaps and gaps.
 * Empty or non-polygonal inputs are removed.
 * Linework is snapped together to eliminate small discrepancies.
 * Overlaps are merged with an adjacent polygon, according to a given merge strategy.
 * Gaps narrower than a given width are filled and merged with an adjacent polygon.
 * The output is an array of polygonal geometries forming a valid polygonal coverage.
 *
 * ** Snapping **
 *
 * Snapping to nearby vertices and line segment snapping
 * is used to improve noding robustness
 * and eliminate small errors in an efficient way,
 * By default this uses a very small snapping distance
 * based on the extent of the input data.
 * The snapping distance may be specified explicitly.
 * This can reduce the number of overlaps and gaps that need to be merged,
 * and reduce the risk of spikes formed by merging gaps.
 * However, a large snapping distance may introduce undesirable
 * data alteration.
 *
 * ** Overlap Merging **
 *
 * Overlaps are merged with an adjacent polygon chosen according to a specified merge strategy.
 * The supported strategies are:
 *
 *    * **Longest Border**: (default) merge with the polygon with longest shared border (#MERGE_LONGEST_BORDER.)
 *    * **Maximum/Minimum Area**: merge with the polygon with largest or smallest area (#MERGE_MAX_AREA, #MERGE_MIN_AREA.)
 *    * **Minimum Index**: merge with the polygon with the lowest index in the input array (#MERGE_MIN_INDEX.)
 *
 * This allows sorting the input according to some criteria to provide a priority
 * for merging gaps.
 *
 * ** Gap Merging **
 *
 * Gaps which are wider than a given distance are merged with an adjacent polygon.
 * Polygon width is determined as twice the radius of the MaximumInscribedCircle
 * of the gap polygon.
 * Gaps are merged with the adjacent polygon with longest shared border.
 * Empty holes in input polygons are treated as gaps, and may be filled in.
 * Gaps which are not fully enclosed ("inlets") are not removed.
 *
 * Cleaning can be run on a valid coverage to remove gaps.
 *
 *
 * The clean result is an array of polygonal geometries
 * which match one-to-one with the input array.
 * A result item may be <tt>null</tt> if:
 *
 *    * the input item is non-polygonal or empty
 *    * the input item is so small it is snapped to collapse
 *    * the input item is covered by another input item
 *        (which may be a larger or a duplicate (nearly or exactly) geometry)
 *
 * The result is a valid coverage according to CoverageValidator#isValid();
 *
 * ** Known Issues **
 *
 *    * Long narrow gaps adjacent to multiple polygons may form spikes when merged with a single polygon.
 *
 * ** Future Enhancements **
 *
 *    * Provide an area-based tolerance for gap merging
 *    * Prevent long narrow gaps from forming spikes by partitioning them before merging.
 *    * Allow merging narrow parts of a gap while leaving wider portions.
 *    * Support a priority value for each input polygon to control overlap and gap merging
 *        (this could also allow blocking polygons from being merge targets)
 *
 * @see CoverageValidator
 * @author Martin Davis
 *
 */
class GEOS_DLL CoverageCleaner {

    using Coordinate = geos::geom::Coordinate;
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Geometry = geos::geom::Geometry;
    using GeometryFactory = geos::geom::GeometryFactory;
    using Polygon = geos::geom::Polygon;
    using LineString = geos::geom::LineString;
    using LinearRing = geos::geom::LinearRing;

public:

    /** Merge strategy that chooses polygon with longest common border */
    static constexpr int MERGE_LONGEST_BORDER = 0;
    /** Merge strategy that chooses polygon with maximum area */
    static constexpr int MERGE_MAX_AREA             = 1;
    /** Merge strategy that chooses polygon with minimum area */
    static constexpr int MERGE_MIN_AREA             = 2;
    /** Merge strategy that chooses polygon with smallest input index */
    static constexpr int MERGE_MIN_INDEX            = 3;

private:

    std::vector<const Geometry*> coverage;
    double snappingDistance;    // set to compute default
    double gapMaximumWidth = 0.0;
    int overlapMergeStrategy = MERGE_LONGEST_BORDER;

    const GeometryFactory* geomFactory;
    std::unique_ptr<STRtree> covIndex;
    std::vector<std::unique_ptr<Polygon>> resultants;
    std::unique_ptr<CleanCoverage> cleanCov;
    std::map<std::size_t, std::vector<std::size_t>> overlapParentMap;
    std::unique_ptr<const Polygon*> overlaps;
    std::unique_ptr<const Polygon*> gaps;
    std::unique_ptr<const Polygon*> mergableGaps;

    static constexpr double DEFAULT_SNAPPING_FACTOR = 1.0e8;

public:





};

} // namespace geos.coverage
} // namespace geos





