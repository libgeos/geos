/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/noding/BasicSegmentString.h>
#include <geos/geom/LineSegment.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/coverage/CoverageRing.h>

#include <unordered_map>
#include <map>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Envelope;
class Geometry;
class GeometryFactory;
}
}

using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineSegment;
using geos::algorithm::locate::IndexedPointInAreaLocator;

namespace geos {      // geos
namespace coverage { // geos::coverage


/**
 * Validates that a polygon forms a valid polygonal coverage
 * with the set of polygons surrounding it.
 * If the polygon is coverage-valid an empty {@link geos::geom::LineString} is returned.
 * Otherwise, the result is a linear geometry containing
 * the polygon boundary linework causing the invalidity.
 *
 * A polygon is coverage-valid if:
 *
 *   * The polygon interior does not intersect the interior of other polygons.
 *   * If the polygon boundary intersects another polygon boundary, the vertices
 *     and line segments of the intersection match exactly.
 *
 * The algorithm detects the following coverage errors:
 *
 *   * the polygon is a duplicate of another one
 *   * a polygon boundary segment is collinear with an adjacent segment but not equal to it
 *   * a polygon boundary segment touches an adjacent segment at a non-vertex point
 *   * a polygon boundary segment crosses into an adjacent polygon
 *   * a polygon boundary segment is in the interior of an adjacent polygon
 *
 * If any of these errors is present, the target polygon
 * does not form a valid coverage with the adjacent polygons.
 *
 * The validity rules does not preclude gaps between coverage polygons.
 * However, this class can detect narrow gaps,
 * by specifying a maximum gap width using {@link #setGapWidth(double)}.
 * Note that this will also identify narrow gaps separating disjoint coverage regions,
 * and narrow gores.
 * In some situations it may also produce false positives
 * (i.e. linework identified as part of a gap which is wider than the given width).
 *
 * A polygon may be coverage-valid with respect to
 * a set of surrounding polygons, but the collection as a whole may not
 * form a clean coverage.  For example, the target polygon boundary may be fully matched
 * by adjacent boundary segments, but the adjacent set contains polygons
 * which are not coverage-valid relative to other ones in the set.
 * A coverage is valid only if every polygon in the coverage is coverage-valid.
 * Use CoverageValidator to validate an entire set of polygons.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL CoveragePolygonValidator {

private:

    // Subclass
    class CoverageRingSegment : public LineSegment
    {
        public:

            CoverageRingSegment(
                const Coordinate& p_p0, const Coordinate& p_p1,
                CoverageRing* p_ring, std::size_t p_index)
                : LineSegment(p_p0, p_p1)
                , m_ring(p_ring)
                , m_index(p_index)
            {
                normalize();
            };

            void markValid() {
                m_ring->markValid(m_index);
            };

            struct CoverageRingSegHash {
                std::size_t
                operator() (CoverageRingSegment const* s) const {
                    std::size_t h = std::hash<double>{}(s->p0.x);
                    h ^= (std::hash<double>{}(s->p0.y) << 1);
                    h ^= (std::hash<double>{}(s->p1.x) << 1);
                    return h ^ (std::hash<double>{}(s->p1.y) << 1);
                }
            };

            struct CoverageRingSegEq {
                bool
                operator() (CoverageRingSegment const* lhs, CoverageRingSegment const* rhs) const {
                    return lhs->p0.x == rhs->p0.x
                        && lhs->p0.y == rhs->p0.y
                        && lhs->p1.x == rhs->p1.x
                        && lhs->p1.y == rhs->p1.y;
                }
            };

        private:

            // Members
            CoverageRing* m_ring;
            std::size_t m_index;
    };

    // Members
    const Geometry* targetGeom;
    std::vector<const Geometry*> adjGeoms;
    const GeometryFactory* geomFactory;
    double gapWidth = 0.0;
    std::map<std::size_t, std::unique_ptr<IndexedPointInAreaLocator>> adjPolygonLocators;
    // std::vector<std::unique_ptr<CoverageRing>> coverageRingStore;
    std::deque<CoverageRing> coverageRingStore;
    std::deque<CoverageRingSegment> coverageRingSegmentStore;

    typedef std::unordered_map<CoverageRingSegment*, CoverageRingSegment*, CoverageRingSegment::CoverageRingSegHash, CoverageRingSegment::CoverageRingSegEq> CoverageRingSegmentMap;

    // Declare type as noncopyable
    CoveragePolygonValidator(const CoveragePolygonValidator& other) = delete;
    CoveragePolygonValidator& operator=(const CoveragePolygonValidator& rhs) = delete;

public:

    /**
    * Validates that a polygon is coverage-valid  against the
    * surrounding polygons in a polygonal coverage.
    *
    * @param targetPolygon the polygon to validate
    * @param adjPolygons the adjacent polygons
    * @return a linear geometry containing the segments causing invalidity (if any)
    */
    static std::unique_ptr<Geometry> validate(
        const Geometry* targetPolygon,
        std::vector<const Geometry*>& adjPolygons);

    /**
    * Validates that a polygon is coverage-valid against the
    * surrounding polygons in a polygonal coverage,
    * and forms no gaps narrower than a specified width.
    * <p>
    * The set of surrounding polygons should include all polygons which
    * are within the gap width distance of the target polygon.
    *
    * @param targetPolygon the polygon to validate
    * @param adjPolygons a collection of the adjacent polygons
    * @param gapWidth the maximum width of invalid gaps
    * @return a linear geometry containing the segments causing invalidity (if any)
    */
    static std::unique_ptr<Geometry> validate(
        const Geometry* targetPolygon,
        std::vector<const Geometry*>& adjPolygons,
        double gapWidth);

    /**
    * Create a new validator.
    *
    * If the gap width is specified, the set of surrounding polygons
    * should include all polygons which
    * are within the gap width distance of the target polygon.
    *
    * @param targetPolygon the geometry to validate
    * @param adjPolygons the adjacent polygons in the polygonal coverage
    */
    CoveragePolygonValidator(
        const Geometry* targetPolygon,
        std::vector<const Geometry*>& adjPolygons);

    /**
    * Sets the maximum gap width, if narrow gaps are to be detected.
    *
    * @param p_gapWidth the maximum width of gaps to detect
    */
    void setGapWidth(double p_gapWidth);

    /**
    * Validates the coverage polygon against the set of adjacent polygons
    * in the coverage.
    *
    * @return a linear geometry containing the segments causing invalidity (if any)
    */
    std::unique_ptr<Geometry> validate();

private:

    static std::vector<const Polygon*> extractPolygons(std::vector<const Geometry*>& geoms);

    /* private */
    std::unique_ptr<Geometry> createEmptyResult();

    /**
    * Check if adjacent geoms contains a duplicate of the target.
    * This situation is not detected by segment alignment checking,
    * since all segments are matches.
    * @param geom
    * @param adjPolygons
    * @return
    */
    bool hasDuplicateGeom(const Geometry* geom, std::vector<const Polygon*>& adjPolygons) const;


    /**
    * Marks matched segments as valid.
    * This improves the efficiency of validity testing, since in valid coverages
    * all segments (except exterior ones) will be matched,
    * and hence do not need to be tested further.
    * In fact, the entire target polygon may be marked valid,
    * which allows avoiding all further tests.
    * Segments matched between adjacent polygons are also marked valid,
    * since this prevents them from being detected as misaligned,
    * if this is being done.
    *
    * @param targetRings the target rings
    * @param adjRings the adjacent rings
    * @param targetEnv the tolerance envelope of the target
    */
    void markMatchedSegments(
        std::vector<CoverageRing*>& targetRings,
        std::vector<CoverageRing*>& adjRings,
        const Envelope& targetEnv);

    /**
    * Adds ring segments to the segment map,
    * and detects if they match an existing segment.
    * Matched segments are marked as coverage-valid.
    *
    * @param rings
    * @param envLimit
    * @param segmentMap
    */
    void markMatchedSegments(
        std::vector<CoverageRing*>& rings,
        const Envelope& envLimit,
        CoverageRingSegmentMap& segmentMap);

    CoverageRingSegment* createCoverageRingSegment(
        CoverageRing* ring, std::size_t index);

    void findInvalidInteractingSegments(
        std::vector<CoverageRing*>& targetRings,
        std::vector<CoverageRing*>& adjRings,
        double distanceTolerance);

    void findInteriorSegments(
        std::vector<CoverageRing*>& targetRings,
        std::vector<const Polygon*>& adjPolygons);

    /**
    * Tests if a coordinate is in the interior of some adjacent polygon.
    * Uses the cached Point-In-Polygon indexed locators, for performance.
    *
    * @param p the coordinate to test
    * @param adjPolygons the list of polygons
    * @return true if the point is in the interior
    */
    bool isInteriorVertex(const Coordinate& p,
        std::vector<const Polygon*>& adjPolygons);


    bool polygonContainsPoint(std::size_t index,
        const Polygon* poly, const Coordinate& pt);

    IndexedPointInAreaLocator* getLocator(std::size_t index, const Polygon* poly);

    std::unique_ptr<Geometry> createInvalidLines(std::vector<CoverageRing*>& rings);


};

} // namespace geos::coverage
} // namespace geos








