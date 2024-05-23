/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 Martin Davis <mtnclimb@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Envelope;
class Polygon;
}
}

using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::Polygon;
using geos::algorithm::locate::IndexedPointInAreaLocator;

namespace geos {        // geos
namespace coverage {    // geos::coverage

class GEOS_DLL CoveragePolygon {

    // Members
    const Polygon* m_polygon;
    Envelope polyEnv;
    mutable std::unique_ptr<IndexedPointInAreaLocator> m_locator;

public:
    CoveragePolygon(const Polygon* poly);

    bool intersectsEnv(const Envelope& env) const;
    bool intersectsEnv(const CoordinateXY& p) const;
    bool contains(const CoordinateXY& p) const;
    
private:
    IndexedPointInAreaLocator& getLocator() const;  

};

} // namespace geos::coverage
} // namespace geos

