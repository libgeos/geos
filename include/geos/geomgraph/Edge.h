/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: geomgraph/Edge.java r428 (JTS-1.12+)
 *
 **********************************************************************/


#pragma once

#include <geos/export.h>
#include <string>
#include <cassert>

#include <geos/geomgraph/GraphComponent.h> // for inheritance
#include <geos/geomgraph/Depth.h> // for member
#include <geos/geomgraph/EdgeIntersectionList.h> // for composition
#include <geos/geom/CoordinateSequence.h> // for inlines
#include <geos/geom/Envelope.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class IntersectionMatrix;
class Coordinate;
}
namespace algorithm {
class LineIntersector;
}
namespace geomgraph {
class Node;
class EdgeEndStar;
class Label;
class NodeFactory;
namespace index {
class MonotoneChainEdge;
}
}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

/** The edge component of a geometry graph */
class GEOS_DLL Edge final: public GraphComponent {
    using GraphComponent::updateIM;

private:

    /// Lazily-created, owned by Edge.
    std::unique_ptr<index::MonotoneChainEdge> mce;

    geom::Envelope env;

    Depth depth;

    int depthDelta;   // the change in area depth from the R to L side of this edge

    bool isIsolatedVar;

public:

    void
    testInvariant() const
    {
        assert(pts);
        assert(pts->size() > 1);
    }

    friend std::ostream& operator<< (std::ostream& os, const Edge& el);

    static void updateIM(const Label& lbl, geom::IntersectionMatrix& im);

    /// Externally-set, owned by Edge. FIXME: refuse ownership
    std::unique_ptr<geom::CoordinateSequence> pts;

    EdgeIntersectionList eiList;

    /// Takes ownership of CoordinateSequence
    Edge(geom::CoordinateSequence* newPts, const Label& newLabel);

    /// Takes ownership of CoordinateSequence
    Edge(geom::CoordinateSequence* newPts);

    ~Edge() override;

    size_t
    getNumPoints() const
    {
        return pts->getSize();
    }

    const geom::CoordinateSequence*
    getCoordinates() const
    {
        testInvariant();
        return pts.get();
    }

    const geom::Coordinate&
    getCoordinate(std::size_t i) const
    {
        testInvariant();
        return pts->getAt(i);
    }

    const geom::Coordinate&
    getCoordinate() const
    {
        testInvariant();
        return pts->getAt(0);
    }


    const Depth&
    getDepth() const
    {
        testInvariant();
        return depth;
    }

    /** \brief
     * The depthDelta is the change in depth as an edge is crossed from R to L
     *
     * @return the change in depth as the edge is crossed from R to L
     */
    int
    getDepthDelta() const
    {
        testInvariant();
        return depthDelta;
    }

    void
    setDepthDelta(int newDepthDelta)
    {
        depthDelta = newDepthDelta;
        testInvariant();
    }

    size_t
    getMaximumSegmentIndex() const
    {
        testInvariant();
        return getNumPoints() - 1;
    }

    EdgeIntersectionList&
    getEdgeIntersectionList()
    {
        testInvariant();
        return eiList;
    }

    const EdgeIntersectionList&
    getEdgeIntersectionList() const
    {
        testInvariant();
        return eiList;
    }

    /// \brief
    /// Return this Edge's index::MonotoneChainEdge,
    /// ownership is retained by this object.
    ///
    index::MonotoneChainEdge* getMonotoneChainEdge();

    bool
    isClosed() const
    {
        testInvariant();
        return pts->getAt(0) == pts->getAt(getNumPoints() - 1);
    }

    /** \brief
     * An Edge is collapsed if it is an Area edge and it consists of
     * two segments which are equal and opposite (eg a zero-width V).
     */
     bool isCollapsed() const;

    Edge* getCollapsedEdge();

    void
    setIsolated(bool newIsIsolated)
    {
        isIsolatedVar = newIsIsolated;
        testInvariant();
    }

    bool
    isIsolated() const override
    {
        testInvariant();
        return isIsolatedVar;
    }

    /** \brief
     * Adds EdgeIntersections for one or both
     * intersections found for a segment of an edge to the edge intersection list.
     */
    void addIntersections(algorithm::LineIntersector* li, std::size_t segmentIndex,
                                  std::size_t geomIndex);

    /// Add an EdgeIntersection for intersection intIndex.
    //
    /// An intersection that falls exactly on a vertex of the edge is normalized
    /// to use the higher of the two possible segmentIndexes
    ///
    void addIntersection(algorithm::LineIntersector* li, std::size_t segmentIndex,
                                 std::size_t geomIndex, std::size_t intIndex);

    /// Update the IM with the contribution for this component.
    //
    /// A component only contributes if it has a labelling for both
    /// parent geometries
    ///
    void
    computeIM(geom::IntersectionMatrix& im) override
    {
        updateIM(label, im);
        testInvariant();
    }

    /// return true if the coordinate sequences of the Edges are identical
    bool isPointwiseEqual(const Edge* e) const;

    std::string print() const;

    std::string printReverse() const;

    /**
     * equals is defined to be:
     *
     * e1 equals e2
     * <b>iff</b>
     * the coordinates of e1 are the same or the reverse of the coordinates in e2
     */
    bool equals(const Edge& e) const;

    bool
    equals(const Edge* e) const
    {
        assert(e);
        return equals(*e);
    }

    const geom::Envelope* getEnvelope();
};


//Operators
inline bool
operator==(const Edge& a, const Edge& b)
{
    return a.equals(b);
}

std::ostream& operator<< (std::ostream& os, const Edge& el);


} // namespace geos.geomgraph
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

