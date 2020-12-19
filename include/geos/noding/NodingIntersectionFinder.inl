/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_NODINGINTERSECTIONFINDER_INL
#define GEOS_NODING_NODINGINTERSECTIONFINDER_INL

#include <geos/geom/Coordinate.h>
#include <geos/noding/SegmentString.h>

namespace geos {
namespace noding {

/* private */
INLINE bool
NodingIntersectionFinder::isEndSegment(const SegmentString* segStr, std::size_t index)
{
    if(index == 0) {
        return true;
    }
    if(index >= segStr->size() - 2) {
        return true;
    }
    return false;
}

/* private */
INLINE bool
NodingIntersectionFinder::isInteriorVertexIntersection(
        const geom::Coordinate& p0, const geom::Coordinate& p1,
        bool isEnd0, bool isEnd1)
{
    // Intersections between endpoints are valid nodes, so not reported
    if(isEnd0 && isEnd1) {
        return false;
    }

    if(p0.equals2D(p1)) {
        return true;
    }

    return false;
}

/* private */
INLINE bool
NodingIntersectionFinder::isInteriorVertexIntersection(
        const geom::Coordinate& p00, const geom::Coordinate& p01,
        const geom::Coordinate& p10, const geom::Coordinate& p11,
        bool isEnd00, bool isEnd01, bool isEnd10, bool isEnd11)
{
    if(isInteriorVertexIntersection(p00, p10, isEnd00, isEnd10)) {
        return true;
    }
    if(isInteriorVertexIntersection(p00, p11, isEnd00, isEnd11)) {
        return true;
    }
    if(isInteriorVertexIntersection(p01, p10, isEnd01, isEnd10)) {
        return true;
    }
    if(isInteriorVertexIntersection(p01, p11, isEnd01, isEnd11)) {
        return true;
    }
    return false;
}

}
}

#endif
