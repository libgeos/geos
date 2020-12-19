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
 **********************************************************************
 *
 * Last port: noding/SegmentNode.java 4667170ea (JTS-1.17)
 *
 **********************************************************************/

#ifndef GEOS_NODING_SEGMENTNODE_INL
#define GEOS_NODING_SEGMENTNODE_INL

#include <geos/noding/SegmentNode.h>
#include <geos/geom/Coordinate.h>
#include <geos/noding/SegmentPointComparator.h>

namespace geos {
namespace noding {

INLINE bool
SegmentNode::isEndPoint(unsigned int maxSegmentIndex) const
{
    if(segmentIndex == 0 && ! isInteriorVar) {
        return true;
    }
    if(segmentIndex == maxSegmentIndex) {
        return true;
    }
    return false;
}

/**
 * @return -1 this EdgeIntersection is located before the argument location
 * @return 0 this EdgeIntersection is at the argument location
 * @return 1 this EdgeIntersection is located after the argument location
 */
INLINE int
SegmentNode::compareTo(const SegmentNode& other) const
{
    if(segmentIndex < other.segmentIndex) {
        return -1;
    }
    if(segmentIndex > other.segmentIndex) {
        return 1;
    }

#if GEOS_DEBUG
    std::cerr << setprecision(17) << "compareTo: " << *this << ", " << other << std::endl;
#endif

    if(coord.equals2D(other.coord)) {

#if GEOS_DEBUG
        std::cerr << " Coordinates equal!" << std::endl;
#endif

        return 0;
    }

#if GEOS_DEBUG
    std::cerr << " Coordinates do not equal!" << std::endl;
#endif

    // an exterior node is the segment start point,
    // so always sorts first
    // this guards against a robustness problem
    // where the octants are not reliable
    if (! isInteriorVar) return -1;
    if (! other.isInteriorVar) return 1;

    return SegmentPointComparator::compare(segmentOctant, coord,
                                           other.coord);
}



}
}

#endif

