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
#include <geos/coverage/CoveragePolygon.h>
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
 * with the set of polygons adjacent to it.
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
 *   * a polygon boundary segment equals an adjacent segment (with same orientation).
 *     This determines that the polygons overlap
 *   * a polygon boundary segment is collinear and overlaps an adjacent segment
 *     but is not equal to it
 *   * a polygon boundary segment touches an adjacent segment at a non-vertex point
 *   * a polygon boundary segment crosses into an adjacent polygon
 *   * a polygon boundary segment is in the interior of an adjacent polygon
 *
 * If any of these errors is present, the target polygon
 * does not form a valid coverage with the adjacent polygons.
 *
 * The validity rules do not preclude gaps between coverage polygons.
 * However, this class can detect narrow gaps,
 * by specifying a maximum gap width using {@link #setGapWidth(double)}.
 * Note that this will also identify narrow gaps separating disjoint coverage regions,
 * and narrow gores.
 * In some situations it may also produce false positives
 * (i.e. linework identified as part of a gap which is wider than the given width).
 * To fully identify gaps it maybe necessary to use {@link CoverageUnion} and analyze
 * the holes in the result to see if they are acceptable.
 *
 * A polygon may be coverage-valid with respect to
 * a set of surrounding polygons, but the collection as a whole may not
 * form a clean coverage.  For example, the target polygon boundary may be fully matched
 * by adjacent boundary segments, but the adjacent set contains polygons
 * which are not coverage-valid relative to other ones in the set.
 * A coverage is valid only if every polygon in the coverage is coverage-valid.
 * Use {@link CoverageValidator} to validate an entire set of polygons.
 *
 * The adjacent set may contain polygons which do not intersect the target polygon.
 * These are effectively ignored during validation (but may decrease performance).
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL CoveragePolygonValidator {

private:

    /**
    * Models a segment in a CoverageRing.
    * The segment is normalized so it can be compared with segments
    * in any orientation.
    * Records valid matching segments in a coverage,
    * which must have opposite orientations.
    * Also detects equal segments with identical
    * orientation, and marks them as coverage-invalid.
    */
    class CoverageRingSegment : public LineSegment
    {
        public:

            // Members
            CoverageRing* ringForward;
            std::size_t indexForward;
            CoverageRing* ringOpp;
            std::size_t indexOpp;

            CoverageRingSegment(
                const Coordinate& p_p0, const Coordinate& p_p1,
                CoverageRing* p_ring, std::size_t p_index)
                : LineSegment(p_p0, p_p1)
                , ringForward(nullptr)
                , indexForward(0)
                , ringOpp(nullptr)
                , indexOpp(0)
            {
                if (p_p1.compareTo(p_p0) < 0) {
                    reverse();
                    ringOpp = p_ring;
                    indexOpp = p_index;
                }
                else {
                    ringForward = p_ring;
                    indexForward = p_index;
                }
            };

            void match(const CoverageRingSegment* seg) {
                bool isInvalid = checkInvalid(seg);
                if (isInvalid) {
                    return;
                }
                //-- record the match
                if (ringForward == nullptr) {
                    ringForward = seg->ringForward;
                    indexForward = seg->indexForward;
                }
                else {
                    ringOpp = seg->ringOpp;
                    indexOpp = seg->indexOpp;
                }
                //-- mark ring segments as matched
                ringForward->markMatched(indexForward);
                ringOpp->markMatched(indexOpp);
            }

            bool checkInvalid(const CoverageRingSegment* seg) const {
                if (ringForward != nullptr && seg->ringForward != nullptr) {
                    ringForward->markInvalid(indexForward);
                    seg->ringForward->markInvalid(seg->indexForward);
                    return true;
                }
                if (ringOpp != nullptr && seg->ringOpp != nullptr) {
                    ringOpp->markInvalid(indexOpp);
                    seg->ringOpp->markInvalid(seg->indexOpp);
                    return true;
                }
                return false;
            }

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

    };

    // Members
    const Geometry* targetGeom;
    std::vector<const Geometry*> adjGeoms;
    //std::vector<const Polygon*> m_adjPolygons;
    const GeometryFactory* geomFactory;
    double gapWidth = 0.0;
    std::vector<std::unique_ptr<CoveragePolygon>> m_adjCovPolygons;
    std::deque<CoverageRing> coverageRingStore;
    std::vector<std::unique_ptr<CoordinateSequence>> localCoordinateSequences;
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

    static std::vector<std::unique_ptr<CoveragePolygon>> 
        toCoveragePolygons(const std::vector<const Polygon*> polygons);
    static std::vector<const Polygon*> extractPolygons(std::vector<const Geometry*>& geoms);

    /* private */
    std::unique_ptr<Geometry> createEmptyResult();

    /**
    * Marks matched segments.
    * This improves the efficiency of validity testing, since in valid coverages
    * all segments (except exterior ones) are matched,
    * and hence do not need to be tested further.
    * Segments which are equal and have same orientation
    * are detected and marked invalid.
    * In fact, the entire target polygon may be matched and valid,
    * which allows avoiding further tests.
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
    * Matched segments are marked.
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

    /**
    * Marks invalid target segments which cross an adjacent ring segment,
    * lie partially in the interior of an adjacent ring,
    * or are nearly collinear with an adjacent ring segment up to the distance tolerance
    *
    * @param targetRings the rings with segments to test
    * @param adjRings the adjacent rings
    * @param distanceTolerance the gap distance tolerance, if any
    */
    void markInvalidInteractingSegments(
        std::vector<CoverageRing*>& targetRings,
        std::vector<CoverageRing*>& adjRings,
        double distanceTolerance);

    /**
    * Marks invalid target segments which are fully interior
    * to an adjacent polygon.
    *
    * @param targetRings the rings with segments to test
    * @param adjCovPolygons the adjacent polygons
    */
    void markInvalidInteriorSegments(
        std::vector<CoverageRing*>& targetRings,
        std::vector<std::unique_ptr<CoveragePolygon>>& adjCovPolygons);

    void markInvalidInteriorSection(
        CoverageRing& ring,
        std::size_t iStart, 
        std::size_t iEnd, 
        std::vector<std::unique_ptr<CoveragePolygon>>& adjCovPolygons );

    void markInvalidInteriorSegment(
        CoverageRing& ring, std::size_t i, CoveragePolygon* adjPoly);

    void checkTargetRings(
        std::vector<CoverageRing*>& targetRings,
        std::vector<CoverageRing*>& adjRngs,
        const Envelope& targetEnv);

    std::unique_ptr<Geometry> createInvalidLines(std::vector<CoverageRing*>& rings);

    std::vector<CoverageRing*> createRings(const Geometry* geom);

    std::vector<CoverageRing*> createRings(std::vector<const Polygon*>& polygons);

    void createRings(const Polygon* poly, std::vector<CoverageRing*>& rings);

    void addRing(
        const LinearRing* ring,
        bool isShell,
        std::vector<CoverageRing*>& rings);

    CoverageRing* createRing(const LinearRing* ring, bool isShell);



};

} // namespace geos::coverage
} // namespace geos
