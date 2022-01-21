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


#include <geos/geom/Coordinate.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/tri/TriangulationBuilder.h>


namespace geos {        // geos
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/* public static */
void
TriangulationBuilder::build(TriList<Tri>& triList)
{
    TriangulationBuilder tb(triList);
}

/* private */
TriangulationBuilder::TriangulationBuilder(TriList<Tri>& triList)
{
    for (auto* tri : triList) {
        add(tri);
    }
}

/* private */
Tri*
TriangulationBuilder::find(const Coordinate& p0, const Coordinate& p1) const
{
    const TriEdge e(p0, p1);
    auto it = triMap.find(e);
    if (it == triMap.end()) {
        // not found
        return nullptr;
    }
    else {
        // found
        return it->second;
    }
}

/* private */
void
TriangulationBuilder::add(Tri* tri)
{
    const Coordinate& p0 = tri->getCoordinate(0);
    const Coordinate& p1 = tri->getCoordinate(1);
    const Coordinate& p2 = tri->getCoordinate(2);

    // get adjacent triangles, if any
    Tri* n0 = find(p0, p1);
    Tri* n1 = find(p1, p2);
    Tri* n2 = find(p2, p0);

    tri->setAdjacent(n0, n1, n2);
    addAdjacent(tri, n0, p0, p1);
    addAdjacent(tri, n1, p1, p2);
    addAdjacent(tri, n2, p2, p0);
}

/* private */
void
TriangulationBuilder::addAdjacent(Tri* tri, Tri* adj, const Coordinate& p0, const Coordinate& p1)
{
    /**
     * If adjacent is null, this tri is first one to be recorded for edge
     */
    if (adj == nullptr) {

        triMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(p0, p1),
            std::forward_as_tuple(tri));

        // TriEdge e(p0, p1);
        // triMap.insert(std::make_pair(e, tri));
        return;
    }
    adj->setAdjacent(p1, tri);
}


} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

