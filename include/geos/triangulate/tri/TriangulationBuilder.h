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

#include <geos/triangulate/tri/TriEdge.h>

#include <memory>
#include <unordered_map>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
namespace triangulate {
namespace tri {
class Tri;
class TriList;
}
}
}

using geos::geom::Coordinate;

namespace geos {        // geos.
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/**
 * Builds a triangulation from a set of {@link Tri}s
 * by linking adjacent Tris.
 *
 * @author mdavis
 *
 */
class GEOS_DLL TriangulationBuilder {

private:

    // Members
    std::unordered_map<TriEdge, Tri*, TriEdge::HashCode> triMap;

    Tri* find(const Coordinate& p0, const Coordinate& p1) const;

    void add(Tri* tri);

    void addAdjacent(Tri* tri, Tri* adj, const Coordinate& p0, const Coordinate& p1);


public:

    TriangulationBuilder(TriList& triList);

    /**
    * Builds the triangulation of a set of {@link Tri}s.
    *
    * @param triList the list of Tris
    */
    static void build(TriList& triList);


};







} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

