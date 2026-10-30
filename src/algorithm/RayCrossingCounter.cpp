/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/RayCrossingCounter.java rev. 1.2 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/geom/CircularArc.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Curve.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/geom/SimpleCurve.h>

#include "../util/SimdAvx512.h"

#include <cmath>

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm {

namespace {

#if GEOS_SIMD_AVX512
/*
 * Counts the segments of a linear sequence of npts >= 2 points, following
 * RayCrossingCounter::countSegment exactly, eight segments at a time.
 * Returns true if the point lies on a segment, in which case crossingCount
 * includes only the segments before it.
 */
template<std::size_t S>
GEOS_AVX512_TARGET bool
countSegmentsAvx512(const double* data, std::size_t npts, const CoordinateXY& point,
                    std::size_t& crossingCount)
{
    namespace simd = geos::util::simd::detail;

    const __m512d px = _mm512_set1_pd(point.x);
    const __m512d py = _mm512_set1_pd(point.y);
    // error bound coefficient of CGAlgorithmsDD::orientationIndexFilter
    const __m512d errCoeff = _mm512_set1_pd(3.3306690621773724e-16);
    const __m512d zero = _mm512_setzero_pd();

    simd::SegmentBlocks<S> blocks(data, npts);
    while (blocks.next()) {
        const std::size_t k = blocks.start();
        const __mmask8 lanes = simd::laneMask(blocks.count());
        const __m512d x1 = blocks.x0;
        const __m512d y1 = blocks.y0;
        const __m512d x2 = blocks.x1;
        const __m512d y2 = blocks.y1;

        // skip segments strictly to the left of the point
        const __mmask8 active = lanes & static_cast<__mmask8>(~(
            _mm512_mask_cmp_pd_mask(lanes, x1, px, _CMP_LT_OQ) &
            _mm512_mask_cmp_pd_mask(lanes, x2, px, _CMP_LT_OQ)));
        if (active == 0) {
            continue;
        }

        // point equal to the segment end vertex
        const __mmask8 vertex = _mm512_mask_cmp_pd_mask(active, x2, px, _CMP_EQ_OQ) &
                                _mm512_mask_cmp_pd_mask(active, y2, py, _CMP_EQ_OQ);
        __mmask8 rest = active & static_cast<__mmask8>(~vertex);

        // horizontal segments at the height of the point
        const __mmask8 horiz = _mm512_mask_cmp_pd_mask(rest, y1, py, _CMP_EQ_OQ) &
                               _mm512_mask_cmp_pd_mask(rest, y2, py, _CMP_EQ_OQ);
        __mmask8 on = vertex;
        if (horiz) {
            const __mmask8 swap = _mm512_mask_cmp_pd_mask(horiz, x1, x2, _CMP_GT_OQ);
            const __m512d minx = _mm512_mask_blend_pd(swap, x1, x2);
            const __m512d maxx = _mm512_mask_blend_pd(swap, x2, x1);
            on = static_cast<__mmask8>(on | (_mm512_mask_cmp_pd_mask(horiz, px, minx, _CMP_GE_OQ) &
                                             _mm512_mask_cmp_pd_mask(horiz, px, maxx, _CMP_LE_OQ)));
        }
        rest = static_cast<__mmask8>(rest & ~horiz);

        // non-horizontal segments crossing the height of the point
        const auto cand = static_cast<__mmask8>(
            (_mm512_mask_cmp_pd_mask(rest, y1, py, _CMP_GT_OQ) &
             _mm512_mask_cmp_pd_mask(rest, y2, py, _CMP_LE_OQ)) |
            (_mm512_mask_cmp_pd_mask(rest, y2, py, _CMP_GT_OQ) &
             _mm512_mask_cmp_pd_mask(rest, y1, py, _CMP_LE_OQ)));
        unsigned cross = 0;
        if (cand) {
            // CGAlgorithmsDD::orientationIndexFilter(p1, p2, point); masked so that
            // no floating-point exceptions are raised by lanes that are not tested
            const __m512d detleft = _mm512_maskz_mul_pd(cand,
                _mm512_maskz_sub_pd(cand, x1, px), _mm512_maskz_sub_pd(cand, y2, py));
            const __m512d detright = _mm512_maskz_mul_pd(cand,
                _mm512_maskz_sub_pd(cand, y1, py), _mm512_maskz_sub_pd(cand, x2, px));
            const __m512d det = _mm512_maskz_sub_pd(cand, detleft, detright);
            const __m512d error = _mm512_maskz_mul_pd(cand,
                _mm512_abs_pd(_mm512_maskz_add_pd(cand, detleft, detright)), errCoeff);
            const __mmask8 certain = _mm512_mask_cmp_pd_mask(cand, _mm512_abs_pd(det), error, _CMP_GE_OQ);

            unsigned pos = _mm512_mask_cmp_pd_mask(certain, det, zero, _CMP_GT_OQ);
            unsigned neg = _mm512_mask_cmp_pd_mask(certain, det, zero, _CMP_LT_OQ);
            unsigned collinear = certain & ~(pos | neg);

            // resolve the remaining lanes with extended precision
            unsigned uncertain = cand & static_cast<unsigned>(~certain);
            while (uncertain) {
                const unsigned lane = simd::countTrailingZeros(uncertain);
                const std::size_t i = k + lane;
                const auto& p1 = *reinterpret_cast<const CoordinateXY*>(data + i * S);
                const auto& p2 = *reinterpret_cast<const CoordinateXY*>(data + (i + 1) * S);
                const int sign = CGAlgorithmsDD::orientationIndex(p1, p2, point);
                const unsigned bit = 1u << lane;
                if (sign > 0) {
                    pos |= bit;
                }
                else if (sign < 0) {
                    neg |= bit;
                }
                else {
                    collinear |= bit;
                }
                uncertain &= uncertain - 1;
            }

            // downward segments count a crossing for a negative orientation
            const unsigned down = _mm512_mask_cmp_pd_mask(cand, y2, y1, _CMP_LT_OQ);
            cross = (pos & ~down) | (neg & down);
            on = static_cast<__mmask8>(on | collinear);
        }

        if (on) {
            const unsigned first = simd::countTrailingZeros(on);
            crossingCount += simd::popCount(cross & ((1u << first) - 1u));
            return true;
        }
        crossingCount += simd::popCount(cross);
    }
    return false;
}
#endif

/*
 * Counts the segments of a linear sequence with AVX-512, if available.
 * Returns false if the scalar path must be used.
 */
bool
countSequenceAvx512(const geom::CoordinateSequence& seq, const CoordinateXY& point,
                    std::size_t& crossingCount, bool& isPointOnSegment)
{
#if GEOS_SIMD_AVX512
    // a non-finite point is left to the scalar path, which reports it
    if (!geos::util::simd::detail::useAvx512() || seq.size() < 2 ||
            !std::isfinite(point.x) || !std::isfinite(point.y)) {
        return false;
    }

    const double* data = seq.data();
    switch (geos::util::simd::detail::coordinateStride(seq)) {
        case 2: isPointOnSegment = countSegmentsAvx512<2>(data, seq.size(), point, crossingCount); return true;
        case 3: isPointOnSegment = countSegmentsAvx512<3>(data, seq.size(), point, crossingCount); return true;
        case 4: isPointOnSegment = countSegmentsAvx512<4>(data, seq.size(), point, crossingCount); return true;
    }
#else
    (void) seq; (void) point; (void) crossingCount; (void) isPointOnSegment;
#endif
    return false;
}

} // anonymous namespace

//
// private:
//

//
// protected:
//

//
// public:
//
/*static*/ geom::Location
RayCrossingCounter::locatePointInRing(const geom::CoordinateXY& point,
                                      const geom::CoordinateSequence& ring)
{
    RayCrossingCounter rcc(point);

    if (countSequenceAvx512(ring, point, rcc.crossingCount, rcc.isPointOnSegment)) {
        return rcc.getLocation();
    }

    for(std::size_t i = 1, ni = ring.size(); i < ni; i++) {
        const geom::CoordinateXY& p1 = ring.getAt<geom::CoordinateXY>(i-1);;
        const geom::CoordinateXY& p2 = ring.getAt<geom::CoordinateXY>(i);

        rcc.countSegment(p1, p2);

        if(rcc.isOnSegment()) {
            return rcc.getLocation();
        }
    }
    return rcc.getLocation();
}

/*static*/ geom::Location
RayCrossingCounter::locatePointInRing(const geom::CoordinateXY& point,
                                      const std::vector<const geom::Coordinate*>& ring)
{
    RayCrossingCounter rcc(point);

    for(std::size_t i = 1, ni = ring.size(); i < ni; i++) {
        const geom::Coordinate& p1 = *ring[ i - 1 ];
        const geom::Coordinate& p2 = *ring[ i ];

        rcc.countSegment(p1, p2);

        if(rcc.isOnSegment()) {
            return rcc.getLocation();
        }
    }
    return rcc.getLocation();
}

geom::Location
RayCrossingCounter::locatePointInRing(const geom::CoordinateXY& point,
                                      const geom::Curve& ring)
{
    RayCrossingCounter rcc(point);

    for (std::size_t i = 0; i < ring.getNumCurves(); i++) {
        const geom::SimpleCurve* curve = ring.getCurveN(i);
        rcc.processSequence(*curve->getCoordinatesRO(), !curve->hasCurvedComponents());
    }

    return rcc.getLocation();
}

void
RayCrossingCounter::processSequence(const geom::CoordinateSequence& seq, bool isLinear)
{
    if (isOnSegment()) {
        return;
    }

    if (isLinear) {
        if (countSequenceAvx512(seq, point, crossingCount, isPointOnSegment)) {
            return;
        }

        for(std::size_t i = 1; i < seq.size(); i++) {
            const geom::CoordinateXY& p1 = seq.getAt<geom::CoordinateXY>(i-1);
            const geom::CoordinateXY& p2 = seq.getAt<geom::CoordinateXY>(i);

            countSegment(p1, p2);

            if (isOnSegment()) 	{
                return;
            }
        }
    } else {
        for (std::size_t i = 2; i < seq.size(); i += 2) {
            geom::CircularArc arc(seq, i-2);
            countArc(arc);

            if (isOnSegment()) 	{
                return;
            }
        }
    }
}

void
RayCrossingCounter::countSegment(const geom::CoordinateXY& p1,
                                 const geom::CoordinateXY& p2)
{
    // For each segment, check if it crosses
    // a horizontal ray running from the test point in
    // the positive x direction.

    // check if the segment is strictly to the left of the test point
    if(p1.x < point.x && p2.x < point.x) {
        return;
    }

    // check if the point is equal to the current ring vertex
    if(point.x == p2.x && point.y == p2.y) {
        isPointOnSegment = true;
        return;
    }

    // For horizontal segments, check if the point is on the segment.
    // Otherwise, horizontal segments are not counted.
    if(p1.y == point.y && p2.y == point.y) {
        double minx = p1.x;
        double maxx = p2.x;

        if(minx > maxx) {
            minx = p2.x;
            maxx = p1.x;
        }

        if(point.x >= minx && point.x <= maxx) {
            isPointOnSegment = true;
        }

        return;
    }

    // Evaluate all non-horizontal segments which cross a horizontal ray
    // to the right of the test pt.
    // To avoid double-counting shared vertices, we use the convention that
    // - an upward edge includes its starting endpoint, and excludes its
    //   final endpoint
    // - a downward edge excludes its starting endpoint, and includes its
    //   final endpoint

    if((p1.y > point.y && p2.y <= point.y) || (p2.y > point.y && p1.y <= point.y)) {
        // For an upward edge, orientationIndex will be positive when p1->p2
        // crosses ray. Conversely, downward edges should have negative sign.
        int sign = CGAlgorithmsDD::orientationIndex(p1, p2, point);
        if(sign == 0) {
            isPointOnSegment = true;
            return;
        }

        if(p2.y < p1.y) {
            sign = -sign;
        }

        // The segment crosses the ray if the sign is strictly positive.
        if(sign > 0) {
            crossingCount++;
        }
    }
}

bool
RayCrossingCounter::shouldCountCrossing(const geom::CircularArc& arc, const geom::CoordinateXY& q) {
    const auto& c = arc.getCenter();

    // To avoid double-counting shared vertices, we count an intersection point if
    // a) is in the interior of the arc
    // b) is at the starting point of the arc, and the arc is directed upward at that point
    // c) is at the ending point of the arc is directed downward at that point
    if (q.equals2D(arc.p0())) {
        if (arc.isCCW()) {
            return q.x > c.x || (q.x == c.x && q.y < c.y);
        } else {
            return q.x < c.x || (q.x == c.x && q.y < c.y);
        }
    } else if (q.equals2D(arc.p2())) {
        if (arc.isCCW()) {
            return q.x < c.x || (q.x == c.x && q.y < c.y);
        } else {
            return q.x > c.x || (q.x == c.x && q.y < c.y);
        }
    } else {
        // Ignore rays tangent to the arc interior. This is topologically equivalent
        // to the ray intersecting adjacent upward and downward segments, which would
        // cancel out.
        return q.x != arc.getCenter().x;
    }
}

/// Return an array of 0-2 unique intersection points between an arc and a horizontal
/// ray extending rightward from a point. If fewer than 2 intersection points exist,
/// some Coordinates in the returned array will be equal to CoordinateXY::getNull().
std::array<geom::CoordinateXY, 2>
RayCrossingCounter::pointsIntersectingHorizontalRay(const geom::CircularArc& arc, const geom::CoordinateXY& origin) {
    const auto& c = arc.getCenter();
    const auto& R = arc.getRadius();

    auto dx = std::sqrt(R*R - std::pow(origin.y - c.y, 2) );

    // Find two points where the horizontal line intersects the circle
    // that is coincident with this arc.
    // Problem: because of floating-point errors, these
    // constructed points may not actually like on the circle.
    CoordinateXY intPt1{c.x - dx, origin.y};
    CoordinateXY intPt2{c.x + dx, origin.y};

    // Solution (best we have for now)
    // Snap computed points to points that define the arc
    double eps = 1e-14;

    for (const CoordinateXY& pt : arc ) {
        if (origin.y == pt.y) {
            if (intPt1.distance(pt) < eps) {
                intPt1 = pt;
            }
            if (intPt2.distance(pt) < eps) {
                intPt2 = pt;
            }
        }
    }

    std::array<CoordinateXY, 2> ret { CoordinateXY::getNull(), CoordinateXY::getNull() };

    std::size_t pos = 0;
    if (intPt1.x >= origin.x && arc.containsPointOnCircle(intPt1)) {
        ret[pos++] = intPt1;
    }
    if (intPt2 != intPt1 && intPt2.x >= origin.x && arc.containsPointOnCircle(intPt2)) {
        ret[pos++] = intPt2;
    }

    return ret;
}

void
RayCrossingCounter::countArc(const geom::CircularArc& arc)
{
    // For each arc, check if it crosses
    // a horizontal ray running from the test point in
    // the positive x direction.

    // If the arc is degenerate, process it is two line segments
    if (arc.isLinear()) {
        countSegment(arc.p0(), arc.p1());
        countSegment(arc.p1(), arc.p2());
        return;
    }

    // Check if the arc is strictly to the left of the test point
    geom::Envelope arcEnvelope;
    CircularArcs::expandEnvelope(arcEnvelope, arc.p0(), arc.p1(), arc.p2());

    if (arcEnvelope.getMaxX() < point.x) {
        return;
    }

    // Evaluate all arcs whose envelope is to the right of the test point.
    if (arcEnvelope.getMaxY() >= point.y && arcEnvelope.getMinY() <= point.y) {
        if (arc.containsPoint(point)) {
            isPointOnSegment = true;
            return;
        }

        auto intPts = pointsIntersectingHorizontalRay(arc, point);
        if (!intPts[0].isNull() && shouldCountCrossing(arc, intPts[0])) {
            crossingCount++;
        }
        if (!intPts[1].isNull() && shouldCountCrossing(arc, intPts[1])) {
            crossingCount++;
        }
    }
}

geom::Location
RayCrossingCounter::getLocation() const
{
    if(isPointOnSegment) {
        return geom::Location::BOUNDARY;
    }

    // The point is in the interior of the ring if the number
    // of X-crossings is odd.
    if((crossingCount % 2) == 1) {
        return geom::Location::INTERIOR;
    }

    return geom::Location::EXTERIOR;
}


bool
RayCrossingCounter::isPointInPolygon() const
{
    return getLocation() != geom::Location::EXTERIOR;
}


} // geos::algorithm
} // geos
