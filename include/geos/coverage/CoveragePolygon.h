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

using geos::geom::Coordinate;
using geos::geom::Envelope;
using geos::geom::Polygon;
using geos::algorithm::locate::IndexedPointInAreaLocator;

namespace geos {        // geos
namespace coverage {    // geos::coverage

class GEOS_DLL CoveragePolygon {

    // Members
    const Polygon* polygon;
    Envelope polyEnv;
    std::unique_ptr<IndexedPointInAreaLocator> locator;

public:
    CoveragePolygon(const Polygon* poly);

    bool intersectsEnv(const Envelope env);
    bool intersectsEnv(const Coordinate p);
    bool contains(const Coordinate p);
    
private:
    IndexedPointInAreaLocator* getLocator();  

};

} // namespace geos::coverage
} // namespace geos

