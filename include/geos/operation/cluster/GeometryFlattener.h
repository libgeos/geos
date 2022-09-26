/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences LLC
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
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>

namespace geos  {
namespace operation {
namespace cluster {


class GEOS_DLL GeometryFlattener {
public:
    /** Flatten a geometry such that it contains no nested components (e.g., MultiPolygons within
     *  GeometryCollections, etc.). The flattened geometry will use the most narrow representation
     *  possible (Polygon preferred to MultiPolygon, MultiPolygon preferred to GeometryCollection.)
     *
     * @brief flatten
     * @param g a geometry to be flattened; consumed by the function
     * @return a geometry with no nested components
     */
    static std::unique_ptr<geom::Geometry> flatten(std::unique_ptr<geom::Geometry>&& g);

private:
    static void flatten(std::unique_ptr<geom::Geometry>&& g, std::vector<std::unique_ptr<geom::Geometry>>& components);
};


}
}
}
