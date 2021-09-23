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
#include <geos/triangulate/tri/TriEdge.h>
#include <geos/export.h>

#include <iostream>

namespace geos {        // geos
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/* private */
void
TriEdge::normalize()
{
    if ( p0.compareTo(p1) < 0 ) {
        Coordinate tmp = p0;
        p0 = p1;
        p1 = tmp;
    }
}


/* public */
std::size_t
TriEdge::HashCode::operator()(TriEdge const& te) const
{
    geom::Coordinate::HashCode coordHash;

    std::size_t result = 17;
    result ^= coordHash(te.p0);
    result ^= coordHash(te.p1);
    return result;
}

/* public */
bool
operator == (TriEdge const& te0, TriEdge const& te1)
{
    return te0.p0 == te1.p0 && te0.p1 == te1.p1;
}



std::ostream&
operator<<(std::ostream& os, const TriEdge& te)
{
    os << "LINESTRING (";
    os << te.p0 << ", ";
    os << te.p1 << ")";
    return os;
}


} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos

