/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *
 **********************************************************************
 *
 * Last port: noding/NodedSegmentString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#ifndef GEOS_NODING_NODEDSEGMENTSTRING_INL
#define GEOS_NODING_NODEDSEGMENTSTRING_INL

#include <geos/noding/NodedSegmentString.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/Octant.h>

#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>


namespace geos {
namespace noding {

/*static private*/
INLINE int
NodedSegmentString::safeOctant(const geom::Coordinate& p0, const geom::Coordinate& p1)
{
    if(p0.equals2D(p1)) {
        return 0;
    }
    return Octant::octant(p0, p1);
}


/*public*/
INLINE int
NodedSegmentString::getSegmentOctant(std::size_t index) const
{
    if(index >= size() - 1) {
        return -1;
    }
    return safeOctant(getCoordinate(index), getCoordinate(index + 1));
    //return Octant::octant(getCoordinate(index), getCoordinate(index+1));
}


/*public*/
INLINE void
NodedSegmentString::addIntersections(algorithm::LineIntersector* li,
                                     std::size_t segmentIndex, std::size_t geomIndex)
{
    for(std::size_t i = 0, n = li->getIntersectionNum(); i < n; ++i) {
        addIntersection(li, segmentIndex, geomIndex, i);
    }
}

/*public*/
INLINE void
NodedSegmentString::addIntersection(algorithm::LineIntersector* li,
                                    std::size_t segmentIndex,
                                    std::size_t geomIndex, std::size_t intIndex)
{
    ::geos::ignore_unused_variable_warning(geomIndex);

    const geom::Coordinate& intPt = li->getIntersection(intIndex);
    addIntersection(intPt, segmentIndex);
}

/*public*/
INLINE void
NodedSegmentString::addIntersection(const geom::Coordinate& intPt,
                                    std::size_t segmentIndex)
{
    std::size_t normalizedSegmentIndex = segmentIndex;

    if(segmentIndex > size() - 2) {
        throw util::IllegalArgumentException("SegmentString::addIntersection: SegmentIndex out of range");
    }

    // normalize the intersection point location
    auto nextSegIndex = normalizedSegmentIndex + 1;
    if(nextSegIndex < size()) {
        const geom::Coordinate& nextPt = pts->getAt(nextSegIndex);

        // Normalize segment index if intPt falls on vertex
        // The check for point equality is 2D only -
        // Z values are ignored
        if(intPt.equals2D(nextPt)) {
            normalizedSegmentIndex = nextSegIndex;
        }
    }

    /*
     * Add the intersection point to edge intersection list
     * (unless the node is already known)
     */
    nodeList.add(intPt, normalizedSegmentIndex);
}

}
}

#endif
