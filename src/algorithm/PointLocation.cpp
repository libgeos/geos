/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/PointLocation.java @ 2017-09-04
 *
 **********************************************************************/

#include <cmath>
#include <vector>

#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Location.h>
#include <geos/util/IllegalArgumentException.h>

namespace geos {
namespace algorithm { // geos.algorithm

/* public static */
bool
PointLocation::isOnSegment(const geom::CoordinateXY& p, const geom::CoordinateXY& p0, const geom::CoordinateXY& p1)
{
    //-- test envelope first since it's faster
    if (! geom::Envelope::intersects(p0, p1, p))
        return false;
    //-- handle endpoints and zero-length segments
    if (p.equals2D(p0) || p.equals2D(p1))
        return true;

    // Membership on already-rounded coordinates.  A certain Ozaki LEFT/RIGHT
    // certifies off; STRAIGHT certifies on.  FAILURE means the double
    // determinant cannot certify a side.
    //
    // Do not treat every nonzero FAILURE as on-segment: at large magnitude
    // the filter can fail while |det| is huge and DD correctly separates
    // (LINESTRING (0 0, 1e16 1e16) / POINT (5e15, 5e15+1), |det| ~ 9e15).
    // Only a tiny nonzero det is a binary64 rounding residual (#968:
    // (1,0)-(0,2) with (0.9,0.2), |det| ~ 5.55e-17).  Exact-zero det on a
    // huge segment is the testA trap — fall through to DD.
    const int filt = CGAlgorithmsDD::orientationIndexFilter(
        p0.x, p0.y, p1.x, p1.y, p.x, p.y);
    if (filt == CGAlgorithmsDD::LEFT || filt == CGAlgorithmsDD::RIGHT) {
        return false;
    }
    if (filt == CGAlgorithmsDD::STRAIGHT) {
        return true;
    }

    const double detleft = (p0.x - p.x) * (p1.y - p.y);
    const double detright = (p0.y - p.y) * (p1.x - p.x);
    const double det = detleft - detright;
    // Explicit absolute bound on |2*signed triangle area|.  #968 residuals
    // sit near 1e-16; the large-scale off-line counterexample is ~1e16.
    static constexpr double kOnSegmentDetTol = 1e-12;
    if (det != 0.0 && std::fabs(det) <= kOnSegmentDetTol) {
        return true;
    }
    return Orientation::index(p0, p1, p) == Orientation::COLLINEAR;
}

/* public static */
bool
PointLocation::isOnLine(const geom::CoordinateXY& p, const geom::CoordinateSequence* pt)
{
    std::size_t ptsize = pt->getSize();
    if(ptsize == 0) {
        return false;
    }

    for(std::size_t i = 1; i < ptsize; ++i) {
        if(isOnSegment(p, 
                        pt->getAt<geom::CoordinateXY>(i - 1), 
                        pt->getAt<geom::CoordinateXY>(i))) {
            return true;
        }
    }
    return false;
}

/* public static */
bool
PointLocation::isInRing(const geom::CoordinateXY& p,
                        const std::vector<const geom::Coordinate*>& ring)
{
    return PointLocation::locateInRing(p, ring) != geom::Location::EXTERIOR;
}

/* public static */
bool
PointLocation::isInRing(const geom::CoordinateXY& p,
                        const geom::CoordinateSequence* ring)
{
    return PointLocation::locateInRing(p, *ring) != geom::Location::EXTERIOR;
}

/* public static */
geom::Location
PointLocation::locateInRing(const geom::CoordinateXY& p,
                            const std::vector<const geom::Coordinate*>& ring)
{
    return RayCrossingCounter::locatePointInRing(p, ring);
}

/* public static */
geom::Location
PointLocation::locateInRing(const geom::CoordinateXY& p,
                            const geom::CoordinateSequence& ring)
{
    return RayCrossingCounter::locatePointInRing(p, ring);
}

geom::Location
PointLocation::locateInRing(const geom::CoordinateXY& p,
                            const geom::Curve& ring) {
    return RayCrossingCounter::locatePointInRing(p, ring);
}


} // namespace geos.algorithm
} // namespace geos

