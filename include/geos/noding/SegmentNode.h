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

#pragma once

#include <geos/export.h>
#include <geos/geom/Coordinate.h>
#include <geos/noding/SegmentPointComparator.h>
#include <geos/noding/SegmentString.h>

#include <vector>
#include <iostream>

namespace geos {
namespace noding { // geos.noding

/**
 * \brief
 * Represents an intersection point between two NodedSegmentString.
 *
 * Final class.
 */
class GEOS_DLL SegmentNode {
private:
    int segmentOctant;

    bool isInteriorVar;

public:
    friend std::ostream& operator<< (std::ostream& os, const SegmentNode& n);

    /// the point of intersection (own copy)
    geom::CoordinateXYZM coord;

    /// the index of the containing line segment in the parent edge
    std::size_t segmentIndex;

    /// Construct a node on the given NodedSegmentString
    ///
    /// @param ss the parent NodedSegmentString
    ///
    /// @param nCoord the coordinate of the intersection, will be copied
    ///
    /// @param nSegmentIndex the index of the segment on parent
    ///                      NodedSegmentString
    ///        where the Node is located.
    ///
    /// @param nSegmentOctant
    ///
    template<typename CoordType>
    SegmentNode(const SegmentString& ss,
        const CoordType& nCoord,
        std::size_t nSegmentIndex, int nSegmentOctant)
        : segmentOctant(nSegmentOctant)
        , coord(nCoord)
        , segmentIndex(nSegmentIndex)
    {
        // Number of points in NodedSegmentString is one-more number of segments
        assert(segmentIndex < ss.size());
        isInteriorVar = !coord.equals2D(ss.getCoordinate(segmentIndex));
    }

    ~SegmentNode() {}

    /// \brief
    /// Return true if this Node is *internal* (not on the boundary)
    /// of the corresponding segment. Currently only the *first*
    /// segment endpoint is checked, actually.
    ///
    bool
    isInterior() const
    {
        return isInteriorVar;
    }

    bool isEndPoint(unsigned int maxSegmentIndex) const
    {
        if(segmentIndex == 0 && ! isInteriorVar) {
            return true;
        }
        if(segmentIndex == maxSegmentIndex) {
            return true;
        }
        return false;
    };

    /**
     * @return -1 this EdgeIntersection is located before
     *            the argument location
     * @return 0 this EdgeIntersection is at the argument location
     * @return 1 this EdgeIntersection is located after the
     *           argument location
     */
    int compareTo(const SegmentNode& other) const
    {
        if (segmentIndex < other.segmentIndex) {
            return -1;
        }
        if (segmentIndex > other.segmentIndex) {
            return 1;
        }

        if (coord.equals2D(other.coord)) {

            return 0;
        }

        // an exterior node is the segment start point,
        // so always sorts first
        // this guards against a robustness problem
        // where the octants are not reliable
        if (!isInteriorVar) return -1;
        if (!other.isInteriorVar) return 1;

        return SegmentPointComparator::compare(
            segmentOctant, coord,
            other.coord);
    };

};

// std::ostream& operator<< (std::ostream& os, const SegmentNode& n);

struct GEOS_DLL SegmentNodeLT {
    bool
    operator()(SegmentNode* s1, SegmentNode* s2) const
    {
        return s1->compareTo(*s2) < 0;
    }

    bool
    operator()(const SegmentNode& s1, const SegmentNode& s2) const
    {
        return s1.compareTo(s2) < 0;
    }
};


} // namespace geos.noding
} // namespace geos






