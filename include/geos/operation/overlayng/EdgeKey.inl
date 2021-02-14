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

#include <geos/operation/overlayng/EdgeKey.h>
#include <geos/operation/overlayng/Edge.h>
#include <geos/geom/Coordinate.h>

namespace geos {
namespace operation {
namespace overlayng {

INLINE
EdgeKey::EdgeKey(const Edge* edge)
{
    initPoints(edge);
}

/*private*/
INLINE void
EdgeKey::initPoints(const Edge* edge)
{
    bool direction = edge->direction();
    if (direction) {
        init(edge->getCoordinate(0),
             edge->getCoordinate(1));
    }
    else {
        std::size_t len = edge->size();
        init(edge->getCoordinate(len - 1),
             edge->getCoordinate(len - 2));
    }
}

/*private*/
INLINE void
EdgeKey::init(const geom::Coordinate& p0, const geom::Coordinate& p1)
{
    p0x = p0.x;
    p0y = p0.y;
    p1x = p1.x;
    p1y = p1.y;
}

/*public*/
INLINE int
EdgeKey::compareTo(const EdgeKey* ek) const
{
    if (p0x < ek->p0x) return -1;
    if (p0x > ek->p0x) return 1;
    if (p0y < ek->p0y) return -1;
    if (p0y > ek->p0y) return 1;
    // first points are equal, compare second
    if (p1x < ek->p1x) return -1;
    if (p1x > ek->p1x) return 1;
    if (p1y < ek->p1y) return -1;
    if (p1y > ek->p1y) return 1;
    return 0;
}

/*public*/
INLINE bool
EdgeKey::equals(const EdgeKey* ek) const
{
    return p0x == ek->p0x
           && p0y == ek->p0y
           && p1x == ek->p1x
           && p1y == ek->p1y;
}

INLINE bool
operator<(const EdgeKey& ek1, const EdgeKey& ek2)
{
    return ek1.compareTo(&ek2) < 0;
}

INLINE bool
operator==(const EdgeKey& ek1, const EdgeKey& ek2)
{
    return ek1.equals(&ek2);
}

}
}
}
