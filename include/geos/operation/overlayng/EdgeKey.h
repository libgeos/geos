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

#include <geos/operation/overlayng/OverlayLabel.h>
#include <geos/operation/overlayng/EdgeKey.h>
#include <geos/operation/overlayng/Edge.h>
#include <geos/geom/Coordinate.h>
#include <geos/export.h>



namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng


using geos::geom::Coordinate;

/**
 * A key for sorting and comparing edges in a noded arrangement.
 * Relies on the fact that in a correctly noded arrangement
 * edges are identical (up to direction)
 * iff they have their first segment in common.
 *
 * @author mdavis
 *
 */
class GEOS_DLL EdgeKey {

private:

    // Members
    double p0x;
    double p0y;
    double p1x;
    double p1y;

    // Methods
    void initPoints(const Edge* edge)
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

    void init(const geom::Coordinate& p0, const geom::Coordinate& p1)
    {
        p0x = p0.x;
        p0y = p0.y;
        p1x = p1.x;
        p1y = p1.y;
    }


public:

    EdgeKey(const Edge* edge)
    {
        initPoints(edge);
    }

    int compareTo(const EdgeKey* ek) const
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

    bool equals(const EdgeKey* ek) const
    {
        return p0x == ek->p0x
               && p0y == ek->p0y
               && p1x == ek->p1x
               && p1y == ek->p1y;
    }

    friend bool operator<(const EdgeKey& ek1, const EdgeKey& ek2)
    {
        return ek1.compareTo(&ek2) < 0;
    };

    friend bool operator==(const EdgeKey& ek1, const EdgeKey& ek2)
    {
        return ek1.equals(&ek2);
    };

};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

