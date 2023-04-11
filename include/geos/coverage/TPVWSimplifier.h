/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <queue>

#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/index/VertexSequencePackedRtree.h>
#include <geos/simplify/LinkedLine.h>
#include <geos/coverage/Corner.h>
#include <geos/export.h>


namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Envelope;
class Geometry;
class GeometryFactory;
class LineString;
class MultiLineString;
}
}


using geos::coverage::Corner;
using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::MultiLineString;
using geos::index::VertexSequencePackedRtree;
using geos::index::strtree::TemplateSTRtree;
using geos::simplify::LinkedLine;


namespace geos {
namespace coverage { // geos::coverage


/**
 * Computes a Topology-Preserving Visvalingam-Whyatt simplification
 * of a set of input lines.
 * The simplified lines will contain no more intersections than are present
 * in the original input.
 * Line and ring endpoints are preserved, except for rings
 * which are flagged as "free".
 *
 * The amount of simplification is determined by a tolerance value,
 * which is a non-zero quantity.
 * It is the square root of the area tolerance used
 * in the Visvalingam-Whyatt algorithm.
 * This equates roughly to the maximum
 * distance by which a simplified line can change from the original.
 *
 * @author mdavis
 *
 */
class GEOS_DLL TPVWSimplifier
{

public:

    // Prototype
    class EdgeIndex;

    /* private static */
    class Edge {

    public:

        // Members
        double areaTolerance;
        bool isFreeRing;
        const Envelope* envelope;
        std::size_t nbPts;
        LinkedLine linkedLine;
        VertexSequencePackedRtree vertexIndex;
        std::size_t minEdgeSize;

        /**
         * Creates a new edge.
         * The endpoints of the edge are preserved during simplification,
         * unless it is a ring and the isFreeRing flag is set.
         *
         * @param p_inputLine the line or ring
         * @param p_isFreeRing whether a ring endpoint can be removed
         * @param p_areaTolerance the simplification tolerance
         */
        Edge(const LineString* p_inputLine, bool p_isFreeRing, double p_areaTolerance);

        const Coordinate& getCoordinate(std::size_t index) const;

        const Envelope* getEnvelopeInternal() const;

        std::size_t size() const;

        std::unique_ptr<CoordinateSequence> simplify(EdgeIndex& edgeIndex);

        void createQueue(Corner::PriorityQueue& pq);

        void addCorner(std::size_t i, Corner::PriorityQueue& cornerQueue);

        bool isRemovable(Corner& corner, EdgeIndex& edgeIndex) const;

        /**
         * Tests if any vertices in a line intersect the corner triangle.
         * Uses the vertex spatial index for efficiency.
         *
         * @param corner the corner vertices
         * @param cornerEnv the envelope of the corner
         * @param edge the hull to test
         * @return true if there is an intersecting vertex
         */
        bool hasIntersectingVertex(const Corner& corner,
            const Envelope& cornerEnv,
            const Edge& edge) const;

        std::vector<std::size_t> query(const Envelope& cornerEnv) const;

        /**
         * Removes a corner by removing the apex vertex from the ring.
         * Two new corners are created with apexes
         * at the other vertices of the corner
         * (if they are non-convex and thus removable).
         *
         * @param corner the corner to remove
         * @param cornerQueue the corner queue
         */
        void removeCorner(
            Corner& corner,
            Corner::PriorityQueue& cornerQueue);

    }; // Edge

    class EdgeIndex
    {
        public:

            TemplateSTRtree<const Edge*> index;

            void add(std::vector<Edge>& edges);

            std::vector<const Edge*> query(const Envelope& queryEnv);

    }; // EdgeIndex


    /**
    * Simplifies a set of lines, preserving the topology of the lines.
    *
    * @param lines the lines to simplify
    * @param distanceTolerance the simplification tolerance
    * @return the simplified lines
    */
    static std::unique_ptr<MultiLineString> simplify(
        const MultiLineString* lines,
        double distanceTolerance);

    /**
    * Simplifies a set of lines, preserving the topology of the lines between
    * themselves and a set of linear constraints.
    * The endpoints of lines are preserved.
    * The endpoint of rings are preserved as well, unless
    * the ring is indicated as "free" via a bit flag with the same index.
    *
    * @param lines the lines to simplify
    * @param freeRings flags indicating which ring edges do not have node endpoints
    * @param constraintLines the linear constraints
    * @param distanceTolerance the simplification tolerance
    * @return the simplified lines
    */
    static std::unique_ptr<MultiLineString> simplify(
        const MultiLineString* lines,
        std::vector<bool>& freeRings,
        const MultiLineString* constraintLines,
        double distanceTolerance);

    // Constructor
    TPVWSimplifier(const MultiLineString* lines,
        double distanceTolerance);


private:

    // Members
    const MultiLineString* inputLines;
    std::vector<bool> isFreeRing;
    double areaTolerance;
    const GeometryFactory* geomFactory;
    const MultiLineString* constraintLines;


    // Methods
    void setFreeRingIndices(std::vector<bool>& freeRing);

    void setConstraints(const MultiLineString* constraints);

    std::unique_ptr<MultiLineString> simplify();

    std::vector<Edge> createEdges(
        const MultiLineString* lines,
        std::vector<bool>& freeRing);


}; // TPVWSimplifier


} // geos::coverage
} // geos
