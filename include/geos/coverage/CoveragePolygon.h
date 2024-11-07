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

namespace geos {        // geos
namespace coverage {    // geos::coverage

class GEOS_DLL CoveragePolygon {
    using CoordinateXY = geos::geom::CoordinateXY;
    using Envelope = geos::geom::Envelope;
    using Polygon = geos::geom::Polygon;
    using IndexedPointInAreaLocator = geos::algorithm::locate::IndexedPointInAreaLocator;

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

