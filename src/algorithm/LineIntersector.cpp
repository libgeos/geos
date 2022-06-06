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
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Distance.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/Intersection.h>
#include <geos/algorithm/NotRepresentableException.h>
#include <geos/geom/Coordinate.h>
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
LineIntersector::computeEdgeDistance(const Coordinate& p, const Coordinate& p0, const Coordinate& p1)
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
void
LineIntersector::computeIntersection(const Coordinate& p1, const Coordinate& p2, const Coordinate& p3,
                                     const Coordinate& p4)
{
    inputLines[0][0] = &p1;
    inputLines[0][1] = &p2;
    inputLines[1][0] = &p3;
    inputLines[1][1] = &p4;
    result = computeIntersect(p1, p2, p3, p4);
    //numIntersects++;
}

/*public*/
std::string
LineIntersector::toString() const
{
    auto getCoordString = [](const Coordinate* coord) -> std::string {
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

/*public static*/
double
LineIntersector::interpolateZ(const Coordinate& p,
                              const Coordinate& p1, const Coordinate& p2)
{
#if GEOS_DEBUG
    std::cerr << "LineIntersector::interpolateZ(" << p.toString() << ", " << p1.toString() << ", " << p2.toString() << ")" <<
         std::endl;
#endif

    if(std::isnan(p1.z)) {
#if GEOS_DEBUG
        std::cerr << " p1 do not have a Z" << std::endl;
#endif
        return p2.z; // might be DoubleNotANumber again
    }

    if(std::isnan(p2.z)) {
#if GEOS_DEBUG
        std::cerr << " p2 do not have a Z" << std::endl;
#endif
        return p1.z; // might be DoubleNotANumber again
    }

    if(p == p1) {
#if GEOS_DEBUG
        std::cerr << " p==p1, returning " << p1.z << std::endl;
#endif
        return p1.z;
    }
    if(p == p2) {
#if GEOS_DEBUG
        std::cerr << " p==p2, returning " << p2.z << std::endl;
#endif
        return p2.z;
    }

    //double zgap = fabs(p2.z - p1.z);
    double zgap = p2.z - p1.z;
    if(zgap == 0.0) {
#if GEOS_DEBUG
        std::cerr << " no zgap, returning " << p2.z << std::endl;
#endif
        return p2.z;
    }
    double xoff = (p2.x - p1.x);
    double yoff = (p2.y - p1.y);
    double seglen = (xoff * xoff + yoff * yoff);
    xoff = (p.x - p1.x);
    yoff = (p.y - p1.y);
    double pdist = (xoff * xoff + yoff * yoff);
    double fract = std::sqrt(pdist / seglen);
    double zoff = zgap * fract;
    //double interpolated = p1.z < p2.z ? p1.z+zoff : p1.z-zoff;
    double interpolated = p1.z + zoff;
#if GEOS_DEBUG
    std::cerr << " zgap:" << zgap << " seglen:" << seglen << " pdist:" << pdist
         << " fract:" << fract << " z:" << interpolated << std::endl;
#endif
    return interpolated;

}


/*public*/
void
LineIntersector::computeIntersection(const Coordinate& p, const Coordinate& p1, const Coordinate& p2)
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
LineIntersector::hasIntersection(const Coordinate& p, const Coordinate& p1, const Coordinate& p2)
{
    if(Envelope::intersects(p1, p2, p)) {
        if((Orientation::index(p1, p2, p) == 0) &&
                (Orientation::index(p2, p1, p) == 0)) {
            return true;
        }
    }
    return false;
}

/*private*/
uint8_t
LineIntersector::computeIntersect(const Coordinate& p1, const Coordinate& p2,
                                  const Coordinate& q1, const Coordinate& q2)
{
#if GEOS_DEBUG
    std::cerr << "LineIntersector::computeIntersect called" << std::endl;
    std::cerr << " p1:" << p1.toString() << " p2:" << p2.toString() << " q1:" << q1.toString() << " q2:" << q2.toString() <<
         std::endl;
#endif // GEOS_DEBUG

    isProperVar = false;

    // first try a fast test to see if the envelopes of the lines intersect
    if(!Envelope::intersects(p1, p2, q1, q2)) {
#if GEOS_DEBUG
        std::cerr << " NO_INTERSECTION" << std::endl;
#endif
        return NO_INTERSECTION;
    }

    // for each endpoint, compute which side of the other segment it lies
    // if both endpoints lie on the same side of the other segment,
    // the segments do not intersect
    int Pq1 = Orientation::index(p1, p2, q1);
    int Pq2 = Orientation::index(p1, p2, q2);

    if((Pq1 > 0 && Pq2 > 0) || (Pq1 < 0 && Pq2 < 0)) {
#if GEOS_DEBUG
        std::cerr << " NO_INTERSECTION" << std::endl;
#endif
        return NO_INTERSECTION;
    }

    int Qp1 = Orientation::index(q1, q2, p1);
    int Qp2 = Orientation::index(q1, q2, p2);

    if((Qp1 > 0 && Qp2 > 0) || (Qp1 < 0 && Qp2 < 0)) {
#if GEOS_DEBUG
        std::cerr << " NO_INTERSECTION" << std::endl;
#endif
        return NO_INTERSECTION;
    }

    /**
     * Intersection is collinear if each endpoint lies on the other line.
     */
    bool collinear = Pq1 == 0 && Pq2 == 0 && Qp1 == 0 && Qp2 == 0;
    if(collinear) {
#if GEOS_DEBUG
        std::cerr << " computingCollinearIntersection" << std::endl;
#endif
        return computeCollinearIntersection(p1, p2, q1, q2);
    }

    /*
     * At this point we know that there is a single intersection point
     * (since the lines are not collinear).
     */

    /*
     * Check if the intersection is an endpoint.
     * If it is, copy the endpoint as
     * the intersection point. Copying the point rather than
     * computing it ensures the point has the exact value,
     * which is important for robustness. It is sufficient to
     * simply check for an endpoint which is on the other line,
     * since at this point we know that the inputLines must
     *  intersect.
     */
    Coordinate p;
    double z = DoubleNotANumber;

    if(Pq1 == 0 || Pq2 == 0 || Qp1 == 0 || Qp2 == 0) {

        isProperVar = false;

#if GEOS_DEBUG
        std::cerr << " intersection is NOT proper" << std::endl;
#endif

        /* Check for two equal endpoints.
         * This is done explicitly rather than by the orientation tests
         * below in order to improve robustness.
         *
         * (A example where the orientation tests fail
         *  to be consistent is:
         *
         * LINESTRING ( 19.850257749638203 46.29709338043669,
         * 			20.31970698357233 46.76654261437082 )
         * and
         * LINESTRING ( -48.51001596420236 -22.063180333403878,
         * 			19.850257749638203 46.29709338043669 )
         *
         * which used to produce the INCORRECT result:
         * (20.31970698357233, 46.76654261437082, NaN)
         */

        if (p1.equals2D(q1)) {
            p = p1;
            z = zGet(p1, q1);
        }
        else if (p1.equals2D(q2)) {
            p = p1;
            z = zGet(p1, q2);
        }
        else if (p2.equals2D(q1)) {
            p = p2;
            z = zGet(p2, q1);
        }
        else if (p2.equals2D(q2)) {
            p = p2;
            z = zGet(p2, q2);
        }
        /*
         * Now check to see if any endpoint lies on the interior of the other segment.
         */
        else if(Pq1 == 0) {
            p = q1;
            z = zGetOrInterpolate(q1, p1, p2);
        }
        else if(Pq2 == 0) {
            p = q2;
            z = zGetOrInterpolate(q2, p1, p2);
        }
        else if(Qp1 == 0) {
            p = p1;
            z = zGetOrInterpolate(p1, q1, q2);
        }
        else if(Qp2 == 0) {
            p = p2;
            z = zGetOrInterpolate(p2, q1, q2);
        }
    }
    else {
#if GEOS_DEBUG
        std::cerr << " intersection is proper" << std::endl;
#endif // GEOS_DEBUG
        isProperVar = true;
        p = intersection(p1, p2, q1, q2);
#if GEOS_DEBUG
        std::cerr << " computed intersection point: " << p << std::endl;
#endif // GEOS_DEBUG
        z = zInterpolate(p, p1, p2, q1, q2);
#if GEOS_DEBUG
        std::cerr << " computed proper intersection Z: " << z << std::endl;
#endif // GEOS_DEBUG
    }
    intPt[0] = Coordinate(p.x, p.y, z);
#if GEOS_DEBUG
    std::cerr << " POINT_INTERSECTION; intPt[0]:" << intPt[0].toString() << std::endl;
#endif // GEOS_DEBUG
    return POINT_INTERSECTION;
}

/*private*/
uint8_t
LineIntersector::computeCollinearIntersection(const Coordinate& p1, const Coordinate& p2,
                                              const Coordinate& q1, const Coordinate& q2)
{

#if GEOS_DEBUG
    std::cerr << "LineIntersector::computeCollinearIntersection called" << std::endl;
    std::cerr << " p1:" << p1.toString() << " p2:" << p2.toString() << " q1:" << q1.toString() << " q2:" << q2.toString() <<
         std::endl;
#endif // GEOS_DEBUG

    bool q1inP = Envelope::intersects(p1, p2, q1);
    bool q2inP = Envelope::intersects(p1, p2, q2);
    bool p1inQ = Envelope::intersects(q1, q2, p1);
    bool p2inQ = Envelope::intersects(q1, q2, p2);

    if(q1inP && q2inP) {
#if GEOS_DEBUG
        std::cerr << " q1inP && q2inP" << std::endl;
#endif
        intPt[0] = zGetOrInterpolateCopy(q1, p1, p2);
        intPt[1] = zGetOrInterpolateCopy(q2, p1, p2);
#if GEOS_DEBUG
        std::cerr << " intPt[0]: " << intPt[0].toString() << std::endl;
        std::cerr << " intPt[1]: " << intPt[1].toString() << std::endl;
#endif
        return COLLINEAR_INTERSECTION;
    }
    if(p1inQ && p2inQ) {
#if GEOS_DEBUG
        std::cerr << " p1inQ && p2inQ" << std::endl;
#endif
        intPt[0] = zGetOrInterpolateCopy(p1, q1, q2);
        intPt[1] = zGetOrInterpolateCopy(p2, q1, q2);
        return COLLINEAR_INTERSECTION;
    }
    if(q1inP && p1inQ) {
#if GEOS_DEBUG
        std::cerr << " q1inP && p1inQ" << std::endl;
#endif
        // if pts are equal Z is chosen arbitrarily
        intPt[0] = zGetOrInterpolateCopy(q1, p1, p2);
        intPt[1] = zGetOrInterpolateCopy(p1, q1, q2);

#if GEOS_DEBUG
        std::cerr << " intPt[0]: " << intPt[0].toString() << std::endl;
        std::cerr << " intPt[1]: " << intPt[1].toString() << std::endl;
#endif
        return (q1 == p1) && !q2inP && !p2inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
    }
    if(q1inP && p2inQ) {
#if GEOS_DEBUG
        std::cerr << " q1inP && p2inQ" << std::endl;
#endif
        // if pts are equal Z is chosen arbitrarily
        intPt[0] = zGetOrInterpolateCopy(q1, p1, p2);
        intPt[1] = zGetOrInterpolateCopy(p2, q1, q2);

#if GEOS_DEBUG
        std::cerr << " intPt[0]: " << intPt[0].toString() << std::endl;
        std::cerr << " intPt[1]: " << intPt[1].toString() << std::endl;
#endif
        return (q1 == p2) && !q2inP && !p1inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
    }
    if(q2inP && p1inQ) {
#if GEOS_DEBUG
        std::cerr << " q2inP && p1inQ" << std::endl;
#endif
        // if pts are equal Z is chosen arbitrarily
        intPt[0] = zGetOrInterpolateCopy(q2, p1, p2);
        intPt[1] = zGetOrInterpolateCopy(p1, q1, q2);
#if GEOS_DEBUG
        std::cerr << " intPt[0]: " << intPt[0].toString() << std::endl;
        std::cerr << " intPt[1]: " << intPt[1].toString() << std::endl;
#endif
        return (q2 == p1) && !q1inP && !p2inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
    }
    if(q2inP && p2inQ) {
#if GEOS_DEBUG
        std::cerr << " q2inP && p2inQ" << std::endl;
#endif
        // if pts are equal Z is chosen arbitrarily
        intPt[0] = zGetOrInterpolateCopy(q2, p1, p2);
        intPt[1] = zGetOrInterpolateCopy(p2, q1, q2);
#if GEOS_DEBUG
        std::cerr << " intPt[0]: " << intPt[0].toString() << std::endl;
        std::cerr << " intPt[1]: " << intPt[1].toString() << std::endl;
#endif
        return (q2 == p2) && !q1inP && !p1inQ ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
    }
    return NO_INTERSECTION;
}

/*private*/
Coordinate
LineIntersector::intersection(const Coordinate& p1, const Coordinate& p2,
                              const Coordinate& q1, const Coordinate& q2) const
{

    Coordinate intPtOut = intersectionSafe(p1, p2, q1, q2);

    /*
     * Due to rounding it can happen that the computed intersection is
     * outside the envelopes of the input segments.  Clearly this
     * is inconsistent.
     * This code checks this condition and forces a more reasonable answer
     *
     * MD - May 4 2005 - This is still a problem.  Here is a failure case:
     *
     * LINESTRING (2089426.5233462777 1180182.3877339689,
     *             2085646.6891757075 1195618.7333999649)
     * LINESTRING (1889281.8148903656 1997547.0560044837,
     *             2259977.3672235999 483675.17050843034)
     * int point = (2097408.2633752143,1144595.8008114607)
     */

    if(! isInSegmentEnvelopes(intPtOut)) {
        //intPt = CentralEndpointIntersector::getIntersection(p1, p2, q1, q2);
        intPtOut = nearestEndpoint(p1, p2, q1, q2);
#if GEOS_DEBUG
        std::cerr << "Intersection outside segment envelopes, snapped to "
             << intPtOut.toString() << std::endl;
#endif
    }

    if(precisionModel != nullptr) {
        precisionModel->makePrecise(intPtOut);
    }

    return intPtOut;
}

/* private static */
double
LineIntersector::zInterpolate(const Coordinate& p, const Coordinate& p1, const Coordinate& p2)
{
#if GEOS_DEBUG
    std::cerr << " zInterpolate " << p << ", " << p1 << ", " << p2 << std::endl;
#endif
    double p1z = p1.z;
    double p2z = p2.z;
    if (std::isnan(p1z)) {
      return p2z; // may be NaN
    }
    if (std::isnan(p2z)) {
      return p1z; // may be NaN
    }
    if (p.equals2D(p1)) {
      return p1z; // not NaN
    }
    if (p.equals2D(p2)) {
      return p2z; // not NaN
    }
    double dz = p2z - p1z;
    if (dz == 0.0) {
      return p1z;
    }
#if GEOS_DEBUG
    std::cerr << " Interpolating Z from distance of p along p1-p2" << std::endl;
#endif
    // interpolate Z from distance of p along p1-p2
    double dx = (p2.x - p1.x);
    double dy = (p2.y - p1.y);
    // seg has non-zero length since p1 < p < p2
    double seglen = (dx * dx + dy * dy);
    double xoff = (p.x - p1.x);
    double yoff = (p.y - p1.y);
    double plen = (xoff * xoff + yoff * yoff);
    double frac = std::sqrt(plen / seglen);
    double zoff = dz * frac;
    double zInterpolated = p1z + zoff;
#if GEOS_DEBUG
    std::cerr << " interpolated Z: " << zInterpolated << std::endl;
#endif
    return zInterpolated;
}



double
LineIntersector::zInterpolate(const Coordinate& p,
                              const Coordinate& p1,
                              const Coordinate& p2,
                              const Coordinate& q1,
                              const Coordinate& q2)
{
#if GEOS_DEBUG
    std::cerr << " zInterpolate(5 coords) called" << std::endl;
#endif // GEOS_DEBUG
    double zp = zInterpolate(p, p1, p2);
#if GEOS_DEBUG
    std::cerr << " zp: " << zp << std::endl;
#endif // GEOS_DEBUG
    double zq = zInterpolate(p, q1, q2);
#if GEOS_DEBUG
    std::cerr << " zq: " << zq << std::endl;
#endif // GEOS_DEBUG
    if (std::isnan(zp)) {
      return zq; // may be NaN
    }
    if (std::isnan(zq)) {
      return zp; // may be NaN
    }
#if GEOS_DEBUG
    std::cerr << " averaging Z" << std::endl;
#endif // GEOS_DEBUG
    // both Zs have values, so average them
    return (zp + zq) / 2.0;
}


/* private static */
Coordinate
LineIntersector::nearestEndpoint(const Coordinate& p1, const Coordinate& p2,
                const Coordinate& q1, const Coordinate& q2)
{
    const Coordinate* nearestPt = &p1;
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

