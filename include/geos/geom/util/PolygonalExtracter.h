/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Geometry.h>
#include <vector>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/**
 * \brief Extracts the polygonal (Polygon and MultiPolygon) elements from a Geometry.
 */
class GEOS_DLL PolygonalExtracter {

public:

    /**
     * Pushes the polygonal (Polygon and MultiPolygon) elements from a geometry into
     * the provided vector.
     * 
     * @param geom the geometry to extract from
     * @param polys the vector to add the polygonal elements to
     */
    static void getPolygonals(const Geometry& geom, std::vector<const Geometry*>& polys);

private:

    static void getPolygonals(const Geometry* geom, std::vector<const Geometry*>& polys);

    // Declare type as noncopyable
    PolygonalExtracter(const PolygonalExtracter& other) = delete;
    PolygonalExtracter& operator=(const PolygonalExtracter& rhs) = delete;
};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

