/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/RobustLineIntersector.java r785 (JTS-1.13+)
 *
 **********************************************************************/

#include <geos/constants.h>
#include <geos/algorithm/Interpolate.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Distance.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/Intersection.h>
#include <geos/algorithm/NotRepresentableException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequences.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Envelope.h>

#include <algorithm> // for max()
#include <string>
#include <sstream>
#include <cmath> // for fabs()
#include <cassert>


#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

/*public static*/
double
LineIntersector::computeEdgeDistance(const CoordinateXY& p, const CoordinateXY& p0, const CoordinateXY& p1)
{
    double dx = fabs(p1.x - p0.x);
    double dy = fabs(p1.y - p0.y);
    double dist = -1.0;	// sentinel value
    if(p == p0) {
        dist = 0.0;
    }
    else if(p == p1) {
        if(dx > dy) {
            dist = dx;
        }
        else {
            dist = dy;
        }
    }
    else {
        double pdx = fabs(p.x - p0.x);
        double pdy = fabs(p.y - p0.y);
        if(dx > dy) {
            dist = pdx;
        }
        else {
            dist = pdy;
        }
        // <FIX>
        // hack to ensure that non-endpoints always have a non-zero distance
        if(dist == 0.0) {
            dist = std::max(pdx, pdy);
        }
    }
    assert(!(dist == 0.0 && !(p == p0))); // Bad distance calculation
    return dist;
}

/*public*/
std::string
LineIntersector::toString() const
{
    auto getCoordString = [](const CoordinateXY* coord) -> std::string {
        return coord ? coord->toString() : "<none>";
    };
    std::ostringstream ss;
    ss << getCoordString(inputLines[0][0]) << "_"
       << getCoordString(inputLines[0][1]) << " "
       << getCoordString(inputLines[1][0]) << "_"
       << getCoordString(inputLines[1][1]) << " : ";
    if(isEndPoint()) {
        ss << " endpoint";
    }
    if(isProperVar) {
        ss << " proper";
    }
    if(isCollinear()) {
        ss << " collinear";
    }
    return ss.str();
}

/*public static*/
bool
LineIntersector::isSameSignAndNonZero(double a, double b)
{
    if(a == 0 || b == 0) {
        return false;
    }
    return (a < 0 && b < 0) || (a > 0 && b > 0);
}

/*private*/
void
LineIntersector::computeIntLineIndex()
{
    computeIntLineIndex(0);
    computeIntLineIndex(1);
}


/*public*/
const Coordinate&
LineIntersector::getIntersectionAlongSegment(std::size_t segmentIndex, std::size_t intIndex)
{
    // lazily compute int line array
    computeIntLineIndex();
    return intPt[intLineIndex[segmentIndex][intIndex]];
}

/*public*/
size_t
LineIntersector::getIndexAlongSegment(std::size_t segmentIndex, std::size_t intIndex)
{
    computeIntLineIndex();
    return intLineIndex[segmentIndex][intIndex];
}

/*private*/
void
LineIntersector::computeIntLineIndex(std::size_t segmentIndex)
{
    double dist0 = getEdgeDistance(segmentIndex, 0);
    double dist1 = getEdgeDistance(segmentIndex, 1);
    if(dist0 > dist1) {
        intLineIndex[segmentIndex][0] = 0;
        intLineIndex[segmentIndex][1] = 1;
    }
    else {
        intLineIndex[segmentIndex][0] = 1;
        intLineIndex[segmentIndex][1] = 0;
    }
}

/*public*/
double
LineIntersector::getEdgeDistance(std::size_t segmentIndex, std::size_t intIndex) const
{
    double dist = computeEdgeDistance(intPt[intIndex],
                                      *inputLines[segmentIndex][0],
                                      *inputLines[segmentIndex][1]);
    return dist;
}

class DoIntersect {
public:
    DoIntersect(algorithm::LineIntersector& li,
                const CoordinateSequence& seq0,
                std::size_t i0,
                const CoordinateSequence& seq1,
                std::size_t i1) :
        m_li(li),
        m_seq0(seq0),
        m_i0(i0),
        m_seq1(seq1),
        m_i1(i1) {}

    template<typename T1, typename T2>
    void operator()() {
        const T1& p00 = m_seq0.getAt<T1>(m_i0);
        const T1& p01 = m_seq0.getAt<T1>(m_i0 + 1);
        const T2& p10 = m_seq1.getAt<T2>(m_i1);
        const T2& p11 = m_seq1.getAt<T2>(m_i1 + 1);

        m_li.computeIntersection(p00, p01, p10, p11);
    }

private:
    algorithm::LineIntersector& m_li;
    const CoordinateSequence& m_seq0;
    std::size_t m_i0;
    const CoordinateSequence& m_seq1;
    std::size_t m_i1;
};

/*public*/
void
LineIntersector::computeIntersection(const CoordinateSequence& p, std::size_t p0,
                                     const CoordinateSequence& q, std::size_t q0)
{
    DoIntersect dis(*this, p, p0, q, q0);
    CoordinateSequences::binaryDispatch(p, q, dis);
}

/*public*/
void
LineIntersector::computeIntersection(const CoordinateXY& p, const CoordinateXY& p1, const CoordinateXY& p2)
{
    isProperVar = false;

    // do between check first, since it is faster than the orientation test
    if(Envelope::intersects(p1, p2, p)) {
        if((Orientation::index(p1, p2, p) == 0) &&
                (Orientation::index(p2, p1, p) == 0)) {
            isProperVar = true;
            if((p == p1) || (p == p2)) { // 2d only test
                isProperVar = false;
            }
            result = POINT_INTERSECTION;
            return;
        }
    }
    result = NO_INTERSECTION;
}

/* public static */
bool
LineIntersector::hasIntersection(const CoordinateXY& p, const CoordinateXY& p1, const CoordinateXY& p2)
{
    if(Envelope::intersects(p1, p2, p)) {
        if((Orientation::index(p1, p2, p) == 0) &&
                (Orientation::index(p2, p1, p) == 0)) {
            return true;
        }
    }
    return false;
}

/* private static */
const CoordinateXY&
LineIntersector::nearestEndpoint(const CoordinateXY& p1, const CoordinateXY& p2,
                                 const CoordinateXY& q1, const CoordinateXY& q2)
{
    const CoordinateXY* nearestPt = &p1;
    double minDist = Distance::pointToSegment(p1, q1, q2);

    double dist = Distance::pointToSegment(p2, q1, q2);
    if(dist < minDist) {
        minDist = dist;
        nearestPt = &p2;
    }
    dist = Distance::pointToSegment(q1, p1, p2);
    if(dist < minDist) {
        minDist = dist;
        nearestPt = &q1;
    }
    dist = Distance::pointToSegment(q2, p1, p2);
    if(dist < minDist) {
        nearestPt = &q2;
    }
    return *nearestPt;
}

} // namespace geos.algorithm
} // namespace geos

