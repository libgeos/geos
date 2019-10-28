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
#include <cmath> // for fabs()
#include <cassert>


#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

#ifndef COMPUTE_Z
#define COMPUTE_Z 1
#endif // COMPUTE_Z

using namespace std;

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm

namespace { // anonymous

/**
 * Finds the endpoint of the segments P and Q which
 * is closest to the other segment.
 * This is a reasonable surrogate for the true
 * intersection points in ill-conditioned cases
 * (e.g. where two segments are nearly coincident,
 * or where the endpoint of one segment lies almost on the other segment).
 * <p>
 * This replaces the older CentralEndpoint heuristic,
 * which chose the wrong endpoint in some cases
 * where the segments had very distinct slopes
 * and one endpoint lay almost on the other segment.
 *
 * @param p1 an endpoint of segment P
 * @param p2 an endpoint of segment P
 * @param q1 an endpoint of segment Q
 * @param q2 an endpoint of segment Q
 * @return the nearest endpoint to the other segment
 */
Coordinate
nearestEndpoint(const Coordinate& p1, const Coordinate& p2,
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
        minDist = dist;
        nearestPt = &q2;
    }
    return *nearestPt;
}


} // anonymous namespace

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
        if(dist == 0.0 && !(p == p0)) {
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
string
LineIntersector::toString() const
{
    string str = inputLines[0][0]->toString() + "_"
                 + inputLines[0][1]->toString() + " "
                 + inputLines[1][0]->toString() + "_"
                 + inputLines[1][1]->toString() + " : ";
    if(isEndPoint()) {
        str += " endpoint";
    }
    if(isProperVar) {
        str += " proper";
    }
    if(isCollinear()) {
        str += " collinear";
    }
    return str;
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
bool
LineIntersector::isIntersection(const Coordinate& pt) const
{
    for(size_t i = 0; i < result; ++i) {
        if(intPt[i].equals2D(pt)) {
            return true;
        }
    }
    return false;
}

/*public*/
const Coordinate&
LineIntersector::getIntersectionAlongSegment(int segmentIndex, int intIndex)
{
    // lazily compute int line array
    computeIntLineIndex();
    return intPt[intLineIndex[segmentIndex][intIndex]];
}

/*public*/
int
LineIntersector::getIndexAlongSegment(int segmentIndex, int intIndex)
{
    computeIntLineIndex();
    return intLineIndex[segmentIndex][intIndex];
}

/*private*/
void
LineIntersector::computeIntLineIndex(int segmentIndex)
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
LineIntersector::getEdgeDistance(size_t segmentIndex, size_t intIndex) const
{
    double dist = computeEdgeDistance(intPt[intIndex],
                                      *inputLines[segmentIndex][0],
                                      *inputLines[segmentIndex][1]);
    return dist;
}

/*public*/
bool
LineIntersector::isInteriorIntersection()
{
    if(isInteriorIntersection(0)) {
        return true;
    }
    if(isInteriorIntersection(1)) {
        return true;
    }
    return false;
}

/*public*/
bool
LineIntersector::isInteriorIntersection(int inputLineIndex)
{
    for(size_t i = 0; i < result; ++i) {
        if(!(intPt[i].equals2D(*inputLines[inputLineIndex][0])
                || intPt[i].equals2D(*inputLines[inputLineIndex][1]))) {
            return true;
        }
    }
    return false;
}

/*public static*/
double
LineIntersector::interpolateZ(const Coordinate& p,
                              const Coordinate& p1, const Coordinate& p2)
{
#if GEOS_DEBUG
    cerr << "LineIntersector::interpolateZ(" << p.toString() << ", " << p1.toString() << ", " << p2.toString() << ")" <<
         endl;
#endif

    if(std::isnan(p1.z)) {
#if GEOS_DEBUG
        cerr << " p1 do not have a Z" << endl;
#endif
        return p2.z; // might be DoubleNotANumber again
    }

    if(std::isnan(p2.z)) {
#if GEOS_DEBUG
        cerr << " p2 do not have a Z" << endl;
#endif
        return p1.z; // might be DoubleNotANumber again
    }

    if(p == p1) {
#if GEOS_DEBUG
        cerr << " p==p1, returning " << p1.z << endl;
#endif
        return p1.z;
    }
    if(p == p2) {
#if GEOS_DEBUG
        cerr << " p==p2, returning " << p2.z << endl;
#endif
        return p2.z;
    }

    //double zgap = fabs(p2.z - p1.z);
    double zgap = p2.z - p1.z;
    if(! zgap) {
#if GEOS_DEBUG
        cerr << " no zgap, returning " << p2.z << endl;
#endif
        return p2.z;
    }
    double xoff = (p2.x - p1.x);
    double yoff = (p2.y - p1.y);
    double seglen = (xoff * xoff + yoff * yoff);
    xoff = (p.x - p1.x);
    yoff = (p.y - p1.y);
    double pdist = (xoff * xoff + yoff * yoff);
    double fract = sqrt(pdist / seglen);
    double zoff = zgap * fract;
    //double interpolated = p1.z < p2.z ? p1.z+zoff : p1.z-zoff;
    double interpolated = p1.z + zoff;
#if GEOS_DEBUG
    cerr << " zgap:" << zgap << " seglen:" << seglen << " pdist:" << pdist
         << " fract:" << fract << " z:" << interpolated << endl;
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
#if COMPUTE_Z
            intPt[0] = p;
#if GEOS_DEBUG
            cerr << "RobustIntersector::computeIntersection(Coordinate,Coordinate,Coordinate) calling interpolateZ" << endl;
#endif
            double z = interpolateZ(p, p1, p2);
            if(!std::isnan(z)) {
                if(std::isnan(intPt[0].z)) {
                    intPt[0].z = z;
                }
                else {
                    intPt[0].z = (intPt[0].z + z) / 2;
                }
            }
#endif // COMPUTE_Z
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
int
LineIntersector::computeIntersect(const Coordinate& p1, const Coordinate& p2,
                                  const Coordinate& q1, const Coordinate& q2)
{
#if GEOS_DEBUG
    cerr << "LineIntersector::computeIntersect called" << endl;
    cerr << " p1:" << p1.toString() << " p2:" << p2.toString() << " q1:" << q1.toString() << " q2:" << q2.toString() <<
         endl;
#endif // GEOS_DEBUG

    isProperVar = false;

    // first try a fast test to see if the envelopes of the lines intersect
    if(!Envelope::intersects(p1, p2, q1, q2)) {
#if GEOS_DEBUG
        cerr << " NO_INTERSECTION" << endl;
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
        cerr << " NO_INTERSECTION" << endl;
#endif
        return NO_INTERSECTION;
    }

    int Qp1 = Orientation::index(q1, q2, p1);
    int Qp2 = Orientation::index(q1, q2, p2);

    if((Qp1 > 0 && Qp2 > 0) || (Qp1 < 0 && Qp2 < 0)) {
#if GEOS_DEBUG
        cerr << " NO_INTERSECTION" << endl;
#endif
        return NO_INTERSECTION;
    }

    bool collinear = Pq1 == 0 && Pq2 == 0 && Qp1 == 0 && Qp2 == 0;
    if(collinear) {
#if GEOS_DEBUG
        cerr << " computingCollinearIntersection" << endl;
#endif
        return computeCollinearIntersection(p1, p2, q1, q2);
    }

    /**
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
    if(Pq1 == 0 || Pq2 == 0 || Qp1 == 0 || Qp2 == 0) {
#if COMPUTE_Z
        int hits = 0;
        double z = 0.0;
#endif
        isProperVar = false;

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
         * which used to produce the result:
         * (20.31970698357233, 46.76654261437082, NaN)
         */

        if(p1.equals2D(q1) || p1.equals2D(q2)) {
            intPt[0] = p1;
#if COMPUTE_Z
            if(!std::isnan(p1.z)) {
                z += p1.z;
                hits++;
            }
#endif
        }
        else if(p2.equals2D(q1) || p2.equals2D(q2)) {
            intPt[0] = p2;
#if COMPUTE_Z
            if(!std::isnan(p2.z)) {
                z += p2.z;
                hits++;
            }
#endif
        }

        /**
         * Now check to see if any endpoint lies on the interior of the other segment.
         */
        else if(Pq1 == 0) {
            intPt[0] = q1;
#if COMPUTE_Z
            if(!std::isnan(q1.z)) {
                z += q1.z;
                hits++;
            }
#endif
        }
        else if(Pq2 == 0) {
            intPt[0] = q2;
#if COMPUTE_Z
            if(!std::isnan(q2.z)) {
                z += q2.z;
                hits++;
            }
#endif
        }
        else if(Qp1 == 0) {
            intPt[0] = p1;
#if COMPUTE_Z
            if(!std::isnan(p1.z)) {
                z += p1.z;
                hits++;
            }
#endif
        }
        else if(Qp2 == 0) {
            intPt[0] = p2;
#if COMPUTE_Z
            if(!std::isnan(p2.z)) {
                z += p2.z;
                hits++;
            }
#endif
        }
#if COMPUTE_Z
#if GEOS_DEBUG
        cerr << "LineIntersector::computeIntersect: z:" << z << " hits:" << hits << endl;
#endif // GEOS_DEBUG
        if(hits) {
            intPt[0].z = z / hits;
        }
#endif // COMPUTE_Z
    }
    else {
        isProperVar = true;
        intPt[0] = intersection(p1, p2, q1, q2);
    }
#if GEOS_DEBUG
    cerr << " POINT_INTERSECTION; intPt[0]:" << intPt[0].toString() << endl;
#endif // GEOS_DEBUG
    return POINT_INTERSECTION;
}

/*private*/
int
LineIntersector::computeCollinearIntersection(const Coordinate& p1, const Coordinate& p2,
                                              const Coordinate& q1, const Coordinate& q2)
{
#if COMPUTE_Z
    double ztot;
    int hits;
    double p2z;
    double p1z;
    double q1z;
    double q2z;
#endif // COMPUTE_Z

#if GEOS_DEBUG
    cerr << "LineIntersector::computeCollinearIntersection called" << endl;
    cerr << " p1:" << p1.toString() << " p2:" << p2.toString() << " q1:" << q1.toString() << " q2:" << q2.toString() <<
         endl;
#endif // GEOS_DEBUG

    bool p1q1p2 = Envelope::intersects(p1, p2, q1);
    bool p1q2p2 = Envelope::intersects(p1, p2, q2);
    bool q1p1q2 = Envelope::intersects(q1, q2, p1);
    bool q1p2q2 = Envelope::intersects(q1, q2, p2);

    if(p1q1p2 && p1q2p2) {
#if GEOS_DEBUG
        cerr << " p1q1p2 && p1q2p2" << endl;
#endif
        intPt[0] = q1;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        q1z = interpolateZ(q1, p1, p2);
        if(!std::isnan(q1z)) {
            ztot += q1z;
            hits++;
        }
        if(!std::isnan(q1.z)) {
            ztot += q1.z;
            hits++;
        }
        if(hits) {
            intPt[0].z = ztot / hits;
        }
#endif
        intPt[1] = q2;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        q2z = interpolateZ(q2, p1, p2);
        if(!std::isnan(q2z)) {
            ztot += q2z;
            hits++;
        }
        if(!std::isnan(q2.z)) {
            ztot += q2.z;
            hits++;
        }
        if(hits) {
            intPt[1].z = ztot / hits;
        }
#endif
#if GEOS_DEBUG
        cerr << " intPt[0]: " << intPt[0].toString() << endl;
        cerr << " intPt[1]: " << intPt[1].toString() << endl;
#endif
        return COLLINEAR_INTERSECTION;
    }
    if(q1p1q2 && q1p2q2) {
#if GEOS_DEBUG
        cerr << " q1p1q2 && q1p2q2" << endl;
#endif
        intPt[0] = p1;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        p1z = interpolateZ(p1, q1, q2);
        if(!std::isnan(p1z)) {
            ztot += p1z;
            hits++;
        }
        if(!std::isnan(p1.z)) {
            ztot += p1.z;
            hits++;
        }
        if(hits) {
            intPt[0].z = ztot / hits;
        }
#endif
        intPt[1] = p2;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        p2z = interpolateZ(p2, q1, q2);
        if(!std::isnan(p2z)) {
            ztot += p2z;
            hits++;
        }
        if(!std::isnan(p2.z)) {
            ztot += p2.z;
            hits++;
        }
        if(hits) {
            intPt[1].z = ztot / hits;
        }
#endif
        return COLLINEAR_INTERSECTION;
    }
    if(p1q1p2 && q1p1q2) {
#if GEOS_DEBUG
        cerr << " p1q1p2 && q1p1q2" << endl;
#endif
        intPt[0] = q1;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        q1z = interpolateZ(q1, p1, p2);
        if(!std::isnan(q1z)) {
            ztot += q1z;
            hits++;
        }
        if(!std::isnan(q1.z)) {
            ztot += q1.z;
            hits++;
        }
        if(hits) {
            intPt[0].z = ztot / hits;
        }
#endif
        intPt[1] = p1;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        p1z = interpolateZ(p1, q1, q2);
        if(!std::isnan(p1z)) {
            ztot += p1z;
            hits++;
        }
        if(!std::isnan(p1.z)) {
            ztot += p1.z;
            hits++;
        }
        if(hits) {
            intPt[1].z = ztot / hits;
        }
#endif
#if GEOS_DEBUG
        cerr << " intPt[0]: " << intPt[0].toString() << endl;
        cerr << " intPt[1]: " << intPt[1].toString() << endl;
#endif
        return (q1 == p1) && !p1q2p2 && !q1p2q2 ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
    }
    if(p1q1p2 && q1p2q2) {
#if GEOS_DEBUG
        cerr << " p1q1p2 && q1p2q2" << endl;
#endif
        intPt[0] = q1;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        q1z = interpolateZ(q1, p1, p2);
        if(!std::isnan(q1z)) {
            ztot += q1z;
            hits++;
        }
        if(!std::isnan(q1.z)) {
            ztot += q1.z;
            hits++;
        }
        if(hits) {
            intPt[0].z = ztot / hits;
        }
#endif
        intPt[1] = p2;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        p2z = interpolateZ(p2, q1, q2);
        if(!std::isnan(p2z)) {
            ztot += p2z;
            hits++;
        }
        if(!std::isnan(p2.z)) {
            ztot += p2.z;
            hits++;
        }
        if(hits) {
            intPt[1].z = ztot / hits;
        }
#endif
#if GEOS_DEBUG
        cerr << " intPt[0]: " << intPt[0].toString() << endl;
        cerr << " intPt[1]: " << intPt[1].toString() << endl;
#endif
        return (q1 == p2) && !p1q2p2 && !q1p1q2 ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
    }
    if(p1q2p2 && q1p1q2) {
#if GEOS_DEBUG
        cerr << " p1q2p2 && q1p1q2" << endl;
#endif
        intPt[0] = q2;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        q2z = interpolateZ(q2, p1, p2);
        if(!std::isnan(q2z)) {
            ztot += q2z;
            hits++;
        }
        if(!std::isnan(q2.z)) {
            ztot += q2.z;
            hits++;
        }
        if(hits) {
            intPt[0].z = ztot / hits;
        }
#endif
        intPt[1] = p1;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        p1z = interpolateZ(p1, q1, q2);
        if(!std::isnan(p1z)) {
            ztot += p1z;
            hits++;
        }
        if(!std::isnan(p1.z)) {
            ztot += p1.z;
            hits++;
        }
        if(hits) {
            intPt[1].z = ztot / hits;
        }
#endif
#if GEOS_DEBUG
        cerr << " intPt[0]: " << intPt[0].toString() << endl;
        cerr << " intPt[1]: " << intPt[1].toString() << endl;
#endif
        return (q2 == p1) && !p1q1p2 && !q1p2q2 ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
    }
    if(p1q2p2 && q1p2q2) {
#if GEOS_DEBUG
        cerr << " p1q2p2 && q1p2q2" << endl;
#endif
        intPt[0] = q2;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        q2z = interpolateZ(q2, p1, p2);
        if(!std::isnan(q2z)) {
            ztot += q2z;
            hits++;
        }
        if(!std::isnan(q2.z)) {
            ztot += q2.z;
            hits++;
        }
        if(hits) {
            intPt[0].z = ztot / hits;
        }
#endif
        intPt[1] = p2;
#if COMPUTE_Z
        ztot = 0;
        hits = 0;
        p2z = interpolateZ(p2, q1, q2);
        if(!std::isnan(p2z)) {
            ztot += p2z;
            hits++;
        }
        if(!std::isnan(p2.z)) {
            ztot += p2.z;
            hits++;
        }
        if(hits) {
            intPt[1].z = ztot / hits;
        }
#endif
#if GEOS_DEBUG
        cerr << " intPt[0]: " << intPt[0].toString() << endl;
        cerr << " intPt[1]: " << intPt[1].toString() << endl;
#endif
        return (q2 == p2) && !p1q1p2 && !q1p1q2 ? POINT_INTERSECTION : COLLINEAR_INTERSECTION;
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
        cerr << "Intersection outside segment envelopes, snapped to "
             << intPt.toString() << endl;
#endif
    }

    if(precisionModel != nullptr) {
        precisionModel->makePrecise(intPtOut);
    }


#if COMPUTE_Z
    double ztot = 0;
    double zvals = 0;
    double zp = interpolateZ(intPtOut, p1, p2);
    double zq = interpolateZ(intPtOut, q1, q2);
    if(!std::isnan(zp)) {
        ztot += zp;
        zvals++;
    }
    if(!std::isnan(zq)) {
        ztot += zq;
        zvals++;
    }
    if(zvals) {
        intPtOut.z = ztot / zvals;
    }
#endif // COMPUTE_Z
    return intPtOut;
}

/*private*/
bool
LineIntersector::isInSegmentEnvelopes(const Coordinate& pt) const
{
    Envelope env0(*inputLines[0][0], *inputLines[0][1]);
    Envelope env1(*inputLines[1][0], *inputLines[1][1]);
    return env0.contains(pt) && env1.contains(pt);
}

/*private*/
Coordinate
LineIntersector::intersectionSafe(const Coordinate& p1, const Coordinate& p2,
                                  const Coordinate& q1, const Coordinate& q2) const
{
    Coordinate ptInt = Intersection::intersection(p1, p2, q1, q2);
    if (ptInt.isNull()) {
        ptInt = nearestEndpoint(p1, p2, q1, q2);
    }
    return ptInt;
}


} // namespace geos.algorithm
} // namespace geos
