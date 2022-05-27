/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/triangulate/tri/TriList.h>

#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class GeometryFactory;
class Polygon;
}
}

using geos::geom::Coordinate;
using geos::geom::Polygon;
using geos::geom::GeometryFactory;

namespace geos {        // geos.
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/**
 * A memory-efficient representation of a triangle in a triangulation.
 * Contains three vertices, and links to adjacent Tris for each edge.
 * Tris are constructed independently, and if needed linked
 * into a triangulation using {@link TriangulationBuilder}.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL Tri {

protected:

    // Members
    Coordinate p0;
    Coordinate p1;
    Coordinate p2;

    /**
    * triN is the adjacent triangle across the edge pN - pNN.
    * pNN is the next vertex CW from pN.
    */
    Tri* tri0;
    Tri* tri1;
    Tri* tri2;

private:

    /**
    * Replace triOld with triNew
    *
    * @param triOld
    * @param triNew
    */
    void replace(Tri* triOld, Tri* triNew);
    void remove(TriIndex index);

    /**
    *
    * Order: 0: opp0-adj0 edge, 1: opp0-adj1 edge,
    *  2: opp1-adj0 edge, 3: opp1-adj1 edge
    *
    * @param tri
    * @param index0
    * @param index1
    * @return list of adjactent tris
    */
    std::vector<Tri*> getAdjacentTris(Tri* tri, TriIndex index0, TriIndex index1);

    void setCoordinates(const Coordinate& p0, const Coordinate& p1, const Coordinate& p2);

    void flip(Tri* tri, TriIndex index0, TriIndex index1,
        const Coordinate& adj0, const Coordinate& adj1,
        const Coordinate& opp0, const Coordinate& opp1);


public:

    Tri(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2)
        : p0(c0)
        , p1(c1)
        , p2(c2)
        , tri0(nullptr)
        , tri1(nullptr)
        , tri2(nullptr)
        {};

    void setAdjacent(Tri* p_tri0, Tri* p_tri1, Tri* p_tri2);
    void setAdjacent(const Coordinate& pt, Tri* tri);
    void setTri(TriIndex edgeIndex, Tri* tri);

    /**
    * Interchanges the vertices of this triangle and a neighbor
    * so that their common edge
    * becomes the the other diagonal of the quadrilateral they form.
    * Neighbour triangle links are modified accordingly.
    *
    * @param index the index of the adjacent tri to flip with
    */
    void flip(TriIndex index);

    /**
    * Removes this triangle from a triangulation.
    * All adjacent references and the references to this
    * Tri in the adjacent Tris are set to nullptr.
    */
    void remove();
    void remove(TriList<Tri>& triList);

    void validate();
    void validateAdjacent(TriIndex index);

    std::pair<const Coordinate&, const Coordinate&> getEdge(Tri* neighbor) const;

    const Coordinate& getEdgeStart(TriIndex i) const;
    const Coordinate& getEdgeEnd(TriIndex i) const;

    bool hasCoordinate(const Coordinate& v) const;
    const Coordinate& getCoordinate(TriIndex i) const;

    TriIndex getIndex(const Coordinate& p) const;
    TriIndex getIndex(const Tri* tri) const;

    Tri* getAdjacent(TriIndex i) const;
    bool hasAdjacent(TriIndex i) const;
    bool hasAdjacent() const;
    bool isAdjacent(Tri* tri) const;

    int numAdjacent() const;

    static TriIndex next(TriIndex i);
    static TriIndex prev(TriIndex i);
    static TriIndex oppVertex(TriIndex edgeIndex);
    static TriIndex oppEdge(TriIndex vertexIndex);

    /**
    * Tests if a tri vertex is interior.
    * A vertex of a triangle is interior if it
    * is fully surrounded by other triangles.
    *
    * @param index the vertex index
    * @return true if the vertex is interior
    */
    bool isInteriorVertex(TriIndex index) const;

    bool isBorder() const;
    bool isBoundary(TriIndex index) const;
    Coordinate midpoint(TriIndex edgeIndex) const;

    double getArea() const;
    double getLength() const;
    double getLength(TriIndex i) const;

    std::unique_ptr<Polygon> toPolygon(const GeometryFactory* gf) const;
    static std::unique_ptr<Geometry> toGeometry(std::set<Tri*>& tris, const GeometryFactory* gf);

    friend std::ostream& operator << (std::ostream& os, const Tri&);

};




} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

