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

#include <geos/export.h>
#include <iostream>


// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
}

using geos::geom::Coordinate;

namespace geos {        // geos.
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/**
 * Represents an edge in a {@link Tri},
 * to be used as a key for looking up Tris
 * while building a triangulation.
 * The edge value is normalized to allow lookup
 * of adjacent triangles.
 *
 * @author mdavis
 */
class GEOS_DLL TriEdge {

private:

    void normalize();


public:

    // Members
    Coordinate p0;
    Coordinate p1;

    TriEdge(const Coordinate& a, const Coordinate& b)
        : p0(a)
        , p1(b)
    {
        normalize();
    }

    struct GEOS_DLL HashCode {
        std::size_t operator()(const TriEdge& te) const;
    };

    friend bool operator == (const TriEdge& te0, const TriEdge& te1);

    friend std::ostream& operator << (std::ostream& os, const TriEdge& te);

};




} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

