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

#include <typeinfo>

#include <geos/geom/util/NoOpGeometryOperation.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

std::unique_ptr<Geometry>
NoOpGeometryOperation::edit(const Geometry* geometry,
                            const GeometryFactory* factory)
{
    std::unique_ptr<Geometry> clonedGeom(factory->createGeometry(geometry));
    return clonedGeom;
}


} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos
