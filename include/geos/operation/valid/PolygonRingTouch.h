/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>


#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
namespace operation {
namespace valid {
class PolygonRing;
}
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::geom::Coordinate;

class GEOS_DLL PolygonRingTouch {

private:

    PolygonRing* ring;
    Coordinate touchPt;


public:

    PolygonRingTouch(PolygonRing* p_ring, const Coordinate& p_pt)
        : ring(p_ring)
        , touchPt(p_pt)
        {};

    const Coordinate* getCoordinate() const;

    PolygonRing* getRing() const;

    bool isAtLocation(const Coordinate& pt) const;

};

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

