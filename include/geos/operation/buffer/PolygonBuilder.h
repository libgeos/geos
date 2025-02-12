/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/overlay/PolygonBuilder.java rev. 1.20 (JTS-1.10)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>

#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class Coordinate;
class GeometryFactory;
}
namespace geomgraph {
class EdgeRing;
class Node;
class PlanarGraph;
class DirectedEdge;
}
namespace operation {
namespace buffer {
class MaximalEdgeRing;
class MinimalEdgeRing;
}
}
}

namespace geos {
namespace operation { // geos::operation
namespace buffer { // geos::operation::buffer

/** \brief
 * Forms Polygon out of a graph of geomgraph::DirectedEdge.
 *
 * The edges to use are marked as being in the result Area.
 */
class GEOS_DLL PolygonBuilder {
public:

    PolygonBuilder(const geom::GeometryFactory* newGeometryFactory);

    ~PolygonBuilder();

    /**
     * Add a complete graph.
     * The graph is assumed to contain one or more polygons,
     * possibly with holes.
     */
    void add(geomgraph::PlanarGraph* graph);
    // throw(const TopologyException &)

    /**
     * Add a set of edges and nodes, which form a graph.
     * The graph is assumed to contain one or more polygons,
     * possibly with holes.
     */
    void add(const std::vector<geomgraph::DirectedEdge*>* dirEdges,
             const std::vector<geomgraph::Node*>* nodes);
    // throw(const TopologyException &)

    std::vector<std::unique_ptr<geom::Geometry>> getPolygons();

private:

    const geom::GeometryFactory* geometryFactory;

    std::vector<geomgraph::EdgeRing*> shellList;

    /**
     * For all DirectedEdges in result, form them into MaximalEdgeRings
     *
     * @param maxEdgeRings
     *   Formed MaximalEdgeRings will be pushed to this vector.
     *   Ownership of the elements is transferred to caller.
     */
    void buildMaximalEdgeRings(
        const std::vector<geomgraph::DirectedEdge*>* dirEdges,
        std::vector<MaximalEdgeRing*>& maxEdgeRings);
    // throw(const TopologyException &)

    void buildMinimalEdgeRings(
        std::vector<MaximalEdgeRing*>& maxEdgeRings,
        std::vector<geomgraph::EdgeRing*>& newShellList,
        std::vector<geomgraph::EdgeRing*>& freeHoleList,
        std::vector<MaximalEdgeRing*>& edgeRings);

    /**
     * This method takes a list of MinimalEdgeRings derived from a
     * MaximalEdgeRing, and tests whether they form a Polygon.
     * This is the case if there is a single shell
     * in the list.  In this case the shell is returned.
     * The other possibility is that they are a series of connected
     * holes, in which case no shell is returned.
     *
     * @return the shell geomgraph::EdgeRing, if there is one
     * @return NULL, if all the rings are holes
     */
    geomgraph::EdgeRing* findShell(std::vector<MinimalEdgeRing*>* minEdgeRings);

    /**
     * This method assigns the holes for a Polygon (formed from a list of
     * MinimalEdgeRings) to its shell.
     * Determining the holes for a MinimalEdgeRing polygon serves two
     * purposes:
     *
     *  - it is faster than using a point-in-polygon check later on.
     *  - it ensures correctness, since if the PIP test was used the point
     *    chosen might lie on the shell, which might return an incorrect
     *    result from the PIP test
     */
    void placePolygonHoles(geomgraph::EdgeRing* shell,
                           std::vector<MinimalEdgeRing*>* minEdgeRings);

    /**
     * For all rings in the input list,
     * determine whether the ring is a shell or a hole
     * and add it to the appropriate list.
     * Due to the way the DirectedEdges were linked,
     * a ring is a shell if it is oriented CW, a hole otherwise.
     */
    void sortShellsAndHoles(std::vector<MaximalEdgeRing*>& edgeRings,
                            std::vector<geomgraph::EdgeRing*>& newShellList,
                            std::vector<geomgraph::EdgeRing*>& freeHoleList);

    struct FastPIPRing {
        geomgraph::EdgeRing* edgeRing;
        algorithm::locate::IndexedPointInAreaLocator* pipLocator;
    };

    /** \brief
     * This method determines finds a containing shell for all holes
     * which have not yet been assigned to a shell.
     *
     * Holes which do not lie in any shell are (probably) an eroded element,
     * so are simply discarded.
     */
    void placeFreeHoles(std::vector<FastPIPRing>& newShellList,
                        std::vector<geomgraph::EdgeRing*>& freeHoleList);

    /** \brief
     * Find the innermost enclosing shell geomgraph::EdgeRing containing the
     * argument geomgraph::EdgeRing, if any.
     *
     * The innermost enclosing ring is the <i>smallest</i> enclosing ring.
     * The algorithm used depends on the fact that:
     *
     * ring A contains ring B iff envelope(ring A)
     * contains envelope(ring B)
     *
     * This routine is only safe to use if the chosen point of the hole
     * is known to be properly contained in a shell
     * (which is guaranteed to be the case if the hole does not touch
     * its shell)
     *
     * @return containing geomgraph::EdgeRing, if there is one
     * @return NULL if no containing geomgraph::EdgeRing is found
     */
    geomgraph::EdgeRing* findEdgeRingContaining(geomgraph::EdgeRing* testEr,
            std::vector<FastPIPRing>& newShellList);

    std::vector<std::unique_ptr<geom::Geometry>> computePolygons(
        std::vector<geomgraph::EdgeRing*>& newShellList);

    /**
     * Checks the current set of shells (with their associated holes) to
     * see if any of them contain the point.
     */

};

} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

