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

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <string>
#include <sstream>

// Forward declarations
namespace geos {
namespace geom {
    class Geometry;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng

/**
 * Represents a computed node along with the incident edges on either side of
 * it (if they exist).
 * This captures the information about a node in a geometry component
 * required to determine the component's contribution to the node topology.
 * A node in an area geometry always has edges on both sides of the node.
 * A node in a linear geometry may have one or other incident edge missing, if
 * the node occurs at an endpoint of the line.
 * The edges of an area node are assumed to be provided
 * with CW-shell orientation (as per JTS norm).
 * This must be enforced by the caller.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL NodeSection {
    using CoordinateXY = geos::geom::CoordinateXY;
    using Geometry = geos::geom::Geometry;

private:

    // Members
    bool m_isA;
    int m_dim;
    int m_id;
    int m_ringId;
    const Geometry* m_poly;
    bool m_isNodeAtVertex;
    const CoordinateXY* m_v0;
    const CoordinateXY m_nodePt;
    const CoordinateXY* m_v1;

    // Methods

    static int compareWithNull(const CoordinateXY* v0, const CoordinateXY* v1);

    static int compare(int a, int b);

public:

    NodeSection(
        bool isA,
        int dim,
        int id,
        int ringId,
        const Geometry* poly,
        bool isNodeAtVertex,
        const CoordinateXY* v0,
        const CoordinateXY nodePt,
        const CoordinateXY* v1)
        : m_isA(isA)
        , m_dim(dim)
        , m_id(id)
        , m_ringId(ringId)
        , m_poly(poly)
        , m_isNodeAtVertex(isNodeAtVertex)
        , m_v0(v0)
        , m_nodePt(nodePt)
        , m_v1(v1)
        {};

    NodeSection(const NodeSection* ns)
        : m_isA(ns->isA())
        , m_dim(ns->dimension())
        , m_id(ns->id())
        , m_ringId(ns->ringId())
        , m_poly(ns->getPolygonal())
        , m_isNodeAtVertex(ns->isNodeAtVertex())
        , m_v0(ns->getVertex(0))
        , m_nodePt(ns->nodePt())
        , m_v1(ns->getVertex(1))
        {};

    const CoordinateXY* getVertex(int i) const;

    const CoordinateXY& nodePt() const;

    int dimension() const;

    int id() const;

    int ringId() const;

    /**
    * Gets the polygon this section is part of.
    * Will be null if section is not on a polygon boundary.
    *
    * @return the associated polygon, or null
    */
    const Geometry* getPolygonal() const;

    bool isShell() const;

    bool isArea() const;

    static bool isAreaArea(const NodeSection& a, const NodeSection& b);

    bool isA() const;

    bool isSameGeometry(const NodeSection& ns) const;

    bool isSamePolygon(const NodeSection& ns) const;

    bool isNodeAtVertex() const;

    bool isProper() const;

    static bool isProper(const NodeSection& a, const NodeSection& b);

    std::string toString() const;

    static std::string edgeRep(const CoordinateXY* p0, const CoordinateXY* p1);

    friend std::ostream& operator<<(std::ostream& os, const NodeSection& ns);

    /**
    * Compare node sections by parent geometry, dimension, element id and ring id,
    * and edge vertices.
    * Sections are assumed to be at the same node point.
    */
    int compareTo(const NodeSection& o) const;


};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

