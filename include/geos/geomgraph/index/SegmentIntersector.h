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
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <array>
#include <vector>

#include <geos/geom/Coordinate.h> // for composition

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace algorithm {
class LineIntersector;
}
namespace geomgraph {
class Node;
class Edge;
}
}

namespace geos {
namespace geomgraph { // geos::geomgraph
namespace index { // geos::geomgraph::index

/// \brief Computes the intersection of line segments, and adds the
/// intersection to the edges containing the segments.
class GEOS_DLL SegmentIntersector {

private:

    /**
     * These variables keep track of what types of intersections were
     * found during ALL edges that have been intersected.
     */
    bool hasIntersectionVar;

    bool hasProper;

    bool hasProperInterior;

    /// the proper intersection point found
    geom::Coordinate properIntersectionPoint;

    algorithm::LineIntersector* li;

    bool includeProper;

    bool recordIsolated;

    int numIntersections;

    /// Elements are externally owned
    std::array<std::vector<Node*>*, 2> bdyNodes;

    bool isTrivialIntersection(Edge* e0, std::size_t segIndex0, Edge* e1, std::size_t segIndex1);

    bool isBoundaryPoint(algorithm::LineIntersector* p_li,
                         std::array<std::vector<Node*>*, 2>& tstBdyNodes)
    {
        return isBoundaryPoint(p_li, tstBdyNodes[0]) || isBoundaryPoint(p_li, tstBdyNodes[1]);
    };

    bool isBoundaryPoint(algorithm::LineIntersector* li,
                         std::vector<Node*>* tstBdyNodes);

public:

    static bool isAdjacentSegments(std::size_t i1, size_t i2)
    {
        return (i1 > i2 ? i1 - i2 : i2 - i1) == 1;
    };

    // testing only
    int numTests;

    //SegmentIntersector();

    virtual
    ~SegmentIntersector() {}

    SegmentIntersector(algorithm::LineIntersector* newLi,
                       bool newIncludeProper, bool newRecordIsolated)
        :
        hasIntersectionVar(false),
        hasProper(false),
        hasProperInterior(false),
        li(newLi),
        includeProper(newIncludeProper),
        recordIsolated(newRecordIsolated),
        numIntersections(0),
        bdyNodes{{nullptr, nullptr}},
        numTests(0)
    {}

    /// \brief
    /// Parameters are externally owned.
    /// Make sure they live for the whole lifetime of this object
    void setBoundaryNodes(std::vector<Node*>* bdyNodes0,
                          std::vector<Node*>* bdyNodes1)
    {
        bdyNodes[0] = bdyNodes0;
        bdyNodes[1] = bdyNodes1;
    };

    /*
    * @return the proper intersection point, or <code>null</code>
    * if none was found
    */
    geom::Coordinate& getProperIntersectionPoint()
    {
        return properIntersectionPoint;
    };

    bool hasIntersection() const
    {
        return hasIntersectionVar;
    };

    /**
     * A proper intersection is an intersection which is interior to at least two
     * line segments.  Note that a proper intersection is not necessarily
     * in the interior of the entire Geometry, since another edge may have
     * an endpoint equal to the intersection, which according to SFS semantics
     * can result in the point being on the Boundary of the Geometry.
     */
    bool hasProperIntersection() const
    {
        return hasProper;
    };

    /**
     * A proper interior intersection is a proper intersection which is <b>not</b>
     * contained in the set of boundary nodes set for this SegmentIntersector.
     */
    bool hasProperInteriorIntersection() const
    {
        return hasProperInterior;
    };

    void addIntersections(Edge* e0, std::size_t segIndex0, Edge* e1, std::size_t segIndex1);

    bool getIsDone() const
    {
        return false;
    };


};

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos


#ifdef _MSC_VER
#pragma warning(pop)
#endif
