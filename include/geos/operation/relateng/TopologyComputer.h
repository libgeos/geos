/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/operation/relateng/NodeSections.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>
#include <geos/export.h>

// Forward declarations
namespace geos {
namespace operation {
namespace relateng {
    class NodeSection;
    class RelateGeometry;
    class RelateNode;
    class TopologyPredicate;
}
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng


class GEOS_DLL TopologyComputer {
    using CoordinateXY = geos::geom::CoordinateXY;
    using Location = geos::geom::Location;
    using NodeSection = geos::operation::relateng::NodeSection;
    using NodeSections = geos::operation::relateng::NodeSections;
    using RelateGeometry = geos::operation::relateng::RelateGeometry;
    using RelateNode = geos::operation::relateng::RelateNode;
    using TopologyPredicate = geos::operation::relateng::TopologyPredicate;

private:

    // Members
    TopologyPredicate& predicate;
    RelateGeometry& geomA;
    RelateGeometry& geomB;
    std::map<CoordinateXY, NodeSections*> nodeMap;
    std::deque<std::unique_ptr<NodeSections>> nodeSectionsStore;

    // Methods

    /**
    * Determine a priori partial EXTERIOR topology based on dimensions.
    */
    void initExteriorDims();

    void initExteriorEmpty(bool geomNonEmpty);

    inline RelateGeometry& getGeometry(bool isA) const {
        return isA ? geomA : geomB;
    };

    void updateDim(Location locA, Location locB, int dimension);

    void updateDim(bool isAB, Location loc1, Location loc2, int dimension);

    /**
     * Update topology for an intersection between A and B.
     *
     * @param a the section for geometry A
     * @param b the section for geometry B
     */
    void updateIntersectionAB(const NodeSection* a, const NodeSection* b);

    /**
     * Updates topology for an AB Area-Area crossing node.
     * Sections cross at a node if (a) the intersection is proper
     * (i.e. in the interior of two segments)
     * or (b) if non-proper then whether the linework crosses
     * is determined by the geometry of the segments on either side of the node.
     * In these situations the area geometry interiors intersect (in dimension 2).
     *
     * @param a the section for geometry A
     * @param b the section for geometry B
     */
    void updateAreaAreaCross(const NodeSection* a, const NodeSection* b);

    /**
     * Updates topology for a node at an AB edge intersection.
     *
     * @param a the section for geometry A
     * @param b the section for geometry B
     */
    void updateNodeLocation(const NodeSection* a, const NodeSection* b);

    void addNodeSections(NodeSection* ns0, NodeSection* ns1);

    void addLineEndOnLine(bool isLineA, Location locLineEnd, Location locLine, const CoordinateXY* pt);

    void addLineEndOnArea(bool isLineA, Location locLineEnd, Location locArea, const CoordinateXY* pt);

    /**
     * Updates topology for an area vertex (in Interior or on Boundary)
     * intersecting a point.
     * Note that because the largest dimension of intersecting target is determined,
     * the intersecting point is not part of any other target geometry,
     * and hence its neighbourhood is in the Exterior of the target.
     *
     * @param isAreaA whether the area is the A input
     * @param locArea the location of the vertex in the area
     * @param pt the point at which topology is being updated
     */
    void addAreaVertexOnPoint(bool isAreaA, Location locArea, const CoordinateXY* pt);

    void addAreaVertexOnLine(bool isAreaA, Location locArea, Location locTarget, const CoordinateXY* pt);

    void evaluateNode(NodeSections* nodeSections);

    void evaluateNodeEdges(const RelateNode* node);

    NodeSections* getNodeSections(const CoordinateXY& nodePt);



public:

    TopologyComputer(
        TopologyPredicate& p_predicate,
        RelateGeometry& p_geomA,
        RelateGeometry& p_geomB)
        : predicate(p_predicate)
        , geomA(p_geomA)
        , geomB(p_geomB)
        {
            initExteriorDims();
        };

    int getDimension(bool isA) const;

    bool isAreaArea() const;

    /**
     * Indicates whether the input geometries require self-noding
     * for correct evaluation of specific spatial predicates.
     * Self-noding is required for geometries which may
     * have self-crossing linework.
     * This causes the coordinates of nodes created by
     * crossing segments to be computed explicitly.
     * This ensures that node locations match in situations
     * where a self-crossing and mutual crossing occur at the same logical location.
     * The canonical example is a self-crossing line tested against a single segment     * identical to one of the crossed segments.
     *
     * @return true if self-noding is required
     */
    bool isSelfNodingRequired() const;

    bool isExteriorCheckRequired(bool isA) const;

    bool isResultKnown() const;

    bool getResult() const;

    /**
     * Finalize the evaluation.
     */
    void finish();

    void addIntersection(NodeSection* a, NodeSection* b);

    void addPointOnPointInterior(const CoordinateXY* pt);

    void addPointOnPointExterior(bool isGeomA, const CoordinateXY* pt);

    void addPointOnGeometry(bool isA, Location locTarget, int dimTarget, const CoordinateXY* pt);

    /**
     * Add topology for a line end.
     * The line end point must be "significant";
     * i.e. not contained in an area if the source is a mixed-dimension GC.
     *
     * @param isLineA the input containing the line end
     * @param locLineEnd the location of the line end (Interior or Boundary)
     * @param locTarget the location on the target geometry
     * @param dimTarget the dimension of the interacting target geometry element,
     *    (if any), or the dimension of the target
     * @param pt the line end coordinate
     */
    void addLineEndOnGeometry(bool isLineA, Location locLineEnd, Location locTarget, int dimTarget, const CoordinateXY* pt);

    /**
     * Adds topology for an area vertex interaction with a target geometry element.
     * Assumes the target geometry element has highest dimension
     * (i.e. if the point lies on two elements of different dimension,
     * the location on the higher dimension element is provided.
     * This is the semantic provided by {@link RelatePointLocator}.
     *
     * Note that in a GeometryCollection containing overlapping or adjacent polygons,
     * the area vertex location may be INTERIOR instead of BOUNDARY.
     *
     * @param isAreaA the input that is the area
     * @param locArea the location on the area
     * @param locTarget the location on the target geometry element
     * @param dimTarget the dimension of the target geometry element
     * @param pt the point of interaction
     */
    void addAreaVertex(bool isAreaA, Location locArea, Location locTarget, int dimTarget, const CoordinateXY* pt);

    void addAreaVertexOnArea(bool isAreaA, Location locArea, Location locTarget, const CoordinateXY* pt);

    void evaluateNodes();

    /**
     * Disable copy construction and assignment. Apparently needed to make this
     * class compile under MSVC. (See https://stackoverflow.com/q/29565299)
     */
    TopologyComputer(const TopologyComputer&) = delete;
    TopologyComputer& operator=(const TopologyComputer&) = delete;


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

