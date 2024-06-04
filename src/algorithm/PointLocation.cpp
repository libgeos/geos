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
    //-- handle zero-length segments
    if (p.equals2D(p0))
        return true;
    bool isOnLine = Orientation::COLLINEAR == Orientation::index(p0, p1, p);
    return isOnLine;
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

