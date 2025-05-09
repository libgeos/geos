/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/dissolve/DissolveEdgeGraph.h>
// #include <memory>
// #include <vector>
// #include <stack>
#include <geos/geom/LineString.h>
#include <geos/export.h>


// Forward declarations
namespace geos {
namespace dissolve {
    class DissolveHalfEdge;
}
namespace edgegraph {
    class HalfEdge;
}
namespace geom {
    class CoordinateSequence;
    class Geometry;
    class GeometryFactory;
    // class LineString;
}
}


namespace geos {      // geos.
namespace dissolve {  // geos.dissolve


/**
 * Dissolves the linear components
 * from a collection of Geometry
 * into a set of maximal-length LineString
 * in which every unique segment appears once only.
 * The output linestrings run between node vertices
 * of the input, which are vertices which have
 * either degree 1, or degree 3 or greater.
 *
 * Use cases for dissolving linear components
 * include generalization
 * (in particular, simplifying polygonal coverages),
 * and visualization
 * (in particular, avoiding symbology conflicts when
 * depicting shared polygon boundaries).
 *
 * This class does **not** node the input lines.
 * If there are line segments crossing in the input,
 * they will still cross in the output.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL LineDissolver {

    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Geometry = geos::geom::Geometry;
    using GeometryFactory = geos::geom::GeometryFactory;
    using LineString = geos::geom::LineString;
    using HalfEdge = geos::edgegraph::HalfEdge;

private:

    std::unique_ptr<Geometry> result;
    const GeometryFactory* factory = nullptr;
    DissolveEdgeGraph graph;
    std::vector<std::unique_ptr<LineString>> lines;
    std::stack<HalfEdge*> nodeEdgeStack;
    DissolveHalfEdge* ringStartEdge = nullptr;


    void computeResult();

    void process(HalfEdge* e);

    /**
     * Adds edges around this node to the stack.
     *
     * @param node
     */
    void stackEdges(HalfEdge* node);

    /**
     * For each edge in stack
     * (which must originate at a node)
     * extracts the line it initiates.
     */
    void buildLines();

    /**
     * Updates the tracked ringStartEdge
     * if the given edge has a lower origin
     * (using the standard Coordinate ordering).
     *
     * Identifying the lowest starting node meets two goals:
     *
     *  * It ensures that isolated input rings are created using the original node and orientation
     *  * For isolated rings formed from multiple input linestrings,
     *    it provides a canonical node and orientation for the output
     *    (rather than essentially random, and thus hard to test).
     *
     * @param e
     */
    void updateRingStartEdge(DissolveHalfEdge* e);

    /**
     * Builds a line starting from the given edge.
     * The start edge origin is a node (valence = 1 or >= 3),
     * unless it is part of a pure ring.
     * A pure ring has no other incident lines.
     * In this case the start edge may occur anywhere on the ring.
     *
     * The line is built up to the next node encountered,
     * or until the start edge is re-encountered
     * (which happens if the edges form a ring).
     *
     * @param eStart
     */
    void buildLine(HalfEdge* eStart);

    void buildRing(HalfEdge* eStartRing);

    void addLine(std::unique_ptr<CoordinateSequence>& cs);


public:

    LineDissolver() : result(nullptr) {};

    /**
     * Dissolves the linear components in a geometry.
     *
     * @param g the geometry to dissolve
     * @return the dissolved lines
     */
    static std::unique_ptr<Geometry> dissolve(const Geometry* g);

    /**
     * Adds a Geometry to be dissolved.
     * Any number of geometries may be added by calling this method multiple times.
     * Any type of Geometry may be added.  The constituent linework will be
     * extracted to be dissolved.
     *
     * @param geometry geometry to be line-merged
     */
    void add(const Geometry* geometry);

    /**
     * Adds a collection of Geometries to be processed. May be called multiple times.
     * Any dimension of Geometry may be added; the constituent linework will be
     * extracted.
     *
     * @param geometries the geometries to be line-merged
     */
    void add(std::vector<const Geometry*> geometries);

    void add(const LineString* lineString);

    /**
     * Gets the dissolved result as a MultiLineString.
     *
     * @return the dissolved lines
     */
    std::unique_ptr<Geometry> getResult();

    /**
     * Disable copy construction and assignment. Needed to make this
     * class compile under MSVC, because it has a vector<unique_ptr>
     * as a member. (See https://stackoverflow.com/q/29565299)
     */
    LineDissolver(const LineDissolver&) = delete;
    LineDissolver& operator=(const LineDissolver&) = delete;


};

} // namespace geos.dissolve
} // namespace geos

