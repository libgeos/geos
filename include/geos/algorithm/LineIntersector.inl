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

#ifndef GEOS_ALGORITHM_LINEINTERSECTOR_INL
#define GEOS_ALGORITHM_LINEINTERSECTOR_INL

#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Intersection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>

namespace geos {
namespace algorithm {

/*public*/
INLINE bool
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
INLINE bool
LineIntersector::isInteriorIntersection(std::size_t inputLineIndex)
{
    for(std::size_t i = 0; i < result; ++i) {
        if(!(intPt[i].equals2D(*inputLines[inputLineIndex][0])
             || intPt[i].equals2D(*inputLines[inputLineIndex][1]))) {
            return true;
        }
    }
    return false;
}

/*public*/
INLINE bool
LineIntersector::isIntersection(const geom::Coordinate& pt) const
{
    for(std::size_t i = 0; i < result; ++i) {
        if(intPt[i].equals2D(pt)) {
            return true;
        }
    }
    return false;
}

/*private*/
INLINE bool
LineIntersector::isInSegmentEnvelopes(const geom::Coordinate& pt) const
{
    geom::Envelope env0(*inputLines[0][0], *inputLines[0][1]);
    geom::Envelope env1(*inputLines[1][0], *inputLines[1][1]);
    return env0.contains(pt) && env1.contains(pt);
}

/*private*/
INLINE geom::Coordinate
LineIntersector::intersectionSafe(const geom::Coordinate& p1, const geom::Coordinate& p2,
                                  const geom::Coordinate& q1, const geom::Coordinate& q2) const
{
    geom::Coordinate ptInt = Intersection::intersection(p1, p2, q1, q2);
    if (ptInt.isNull()) {
        ptInt = nearestEndpoint(p1, p2, q1, q2);
    }
    return ptInt;
}

/* private static */
INLINE double
LineIntersector::zGet(const geom::Coordinate& p, const geom::Coordinate& q)
{
    double z = p.z;
    if ( std::isnan(z) ) {
        z = q.z; // may be NaN
    }
    return z;
}
/* private static */
INLINE double
LineIntersector::zGetOrInterpolate(const geom::Coordinate& p, const geom::Coordinate& p1, const geom::Coordinate& p2)
{
    double z = p.z;
    if (! std::isnan(z) ) return z;
    return zInterpolate(p, p1, p2); // may be NaN
}

/* private static */
INLINE geom::Coordinate
LineIntersector::zGetOrInterpolateCopy(
        const geom::Coordinate& p,
        const geom::Coordinate& p1,
        const geom::Coordinate& p2)
{
    geom::Coordinate pCopy = p;
    double z = zGetOrInterpolate(p, p1, p2);
    pCopy.z = z;
    return pCopy;
}

}
}

#endif
