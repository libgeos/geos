/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Paul Ramsey <pramsey@cleverelephant.ca>
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
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
}
}

namespace geos {      // geos
namespace coverage { // geos::coverage

/**
 * Utility to extract unique edges from a polygonal coverage.
 *
 * @author Paul Ramsey
 *
 */
class GEOS_DLL CoverageEdges {
public:

    /**
    * Returns a MultiLineString representing the unique edges of a polygonal coverage.
    *
    * @param coverage a vector of polygons in the coverage
    * @param type selection type: 0 = ALL, 1 = EXTERIOR (non-shared), 2 = INTERIOR (shared)
    * @return a MultiLineString of unique edges
    */
    static std::unique_ptr<geom::Geometry> GetEdges(const std::vector<const geom::Geometry*>& coverage, int type = 0);

    /**
    * Returns a MultiLineString representing the unique edges of a polygonal coverage.
    *
    * @param coverage a GeometryCollection or MultiPolygon representing the coverage
    * @param type selection type: 0 = ALL, 1 = EXTERIOR (non-shared), 2 = INTERIOR (shared)
    * @return a MultiLineString of unique edges
    */
    static std::unique_ptr<geom::Geometry> GetEdges(const geom::Geometry* coverage, int type = 0);

};

} // namespace geos::coverage
} // namespace geos
