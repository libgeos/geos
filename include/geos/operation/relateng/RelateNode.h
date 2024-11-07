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

#include <geos/operation/relateng/RelateEdge.h>

#include <vector>
#include <memory>
#include <cassert>

#include <geos/export.h>


// Forward declarations
namespace geos {
namespace operation {
namespace relateng {
    class NodeSection;
}
}
namespace geom {
    class CoordinateXY;
    class Geometry;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng


class GEOS_DLL RelateNode {
    using CoordinateXY = geos::geom::CoordinateXY;
    using Geometry = geos::geom::Geometry;

private:

    // Members

    /**
    * A list of the edges around the node in CCW order,
    * ordered by their CCW angle with the positive X-axis.
    */
    std::vector<std::unique_ptr<RelateEdge>> edges;

    const CoordinateXY* nodePt;


    // Methods

    void updateEdgesInArea(bool isA, std::size_t indexFrom, std::size_t indexTo);

    void updateIfAreaPrev(bool isA, std::size_t index);

    void updateIfAreaNext(bool isA, std::size_t index);

    const RelateEdge* addLineEdge(bool isA, const CoordinateXY* dirPt);

    const RelateEdge* addAreaEdge(bool isA, const CoordinateXY* dirPt, bool isForward);

    /**
    * Adds or merges an edge to the node.
    *
    * @param isA
    * @param dirPt
    * @param dim dimension of the geometry element containing the edge
    * @param isForward the direction of the edge
    *
    * @return the created or merged edge for this point
    */
    const RelateEdge* addEdge(bool isA, const CoordinateXY* dirPt, int dim, bool isForward);

    void finishNode(bool isA, bool isAreaInterior);

    void propagateSideLocations(bool isA, std::size_t startIndex);

    static std::size_t prevIndex(std::vector<std::unique_ptr<RelateEdge>>& list, std::size_t index);

    static std::size_t nextIndex(std::vector<std::unique_ptr<RelateEdge>>& list, std::size_t i);

    std::size_t indexOf(
        const std::vector<std::unique_ptr<RelateEdge>>& edges,
        const RelateEdge* edge) const;


public:

    RelateNode(const CoordinateXY* pt)
        : nodePt(pt)
        {};

    const CoordinateXY* getCoordinate() const;

    const std::vector<std::unique_ptr<RelateEdge>>& getEdges() const;

    void addEdges(std::vector<const NodeSection *>& nss);
    void addEdges(std::vector<std::unique_ptr<NodeSection>>& nss);

    void addEdges(const NodeSection* ns);

    /**
    * Computes the final topology for the edges around this node.
    * Although nodes lie on the boundary of areas or the interior of lines,
    * in a mixed GC they may also lie in the interior of an area.
    * This changes the locations of the sides and line to Interior.
    *
    * @param isAreaInteriorA true if the node is in the interior of A
    * @param isAreaInteriorB true if the node is in the interior of B
    */
    void finish(bool isAreaInteriorA, bool isAreaInteriorB);

    std::string toString() const;

    bool hasExteriorEdge(bool isA);

    friend std::ostream& operator<<(std::ostream& os, const RelateNode& ns);

    /**
     * Disable copy construction and assignment. Apparently needed to make this
     * class compile under MSVC. (See https://stackoverflow.com/q/29565299)
     */
    RelateNode(const RelateNode&) = delete;
    RelateNode& operator=(const RelateNode&) = delete;

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

