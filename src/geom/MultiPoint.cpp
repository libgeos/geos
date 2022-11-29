/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/MultiPoint.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Dimension.h>

#include <vector>

namespace geos {
namespace geom { // geos::geom

/*protected*/
MultiPoint::MultiPoint(std::vector<std::unique_ptr<Point>> && newPoints, const GeometryFactory& factory)
    :
    GeometryCollection(std::move(newPoints), factory)
{

}

MultiPoint::MultiPoint(std::vector<std::unique_ptr<Geometry>> && newPoints, const GeometryFactory& factory)
        :
        GeometryCollection(std::move(newPoints), factory)
{

}

Dimension::DimensionType
MultiPoint::getDimension() const
{
    return Dimension::P; // point
}

int
MultiPoint::getBoundaryDimension() const
{
    return Dimension::False;
}

std::string
MultiPoint::getGeometryType() const
{
    return "MultiPoint";
}

std::unique_ptr<Geometry>
MultiPoint::getBoundary() const
{
    return std::unique_ptr<Geometry>(getFactory()->createGeometryCollection());
}

const CoordinateXY*
MultiPoint::getCoordinateN(std::size_t n) const
{
    return geometries[n]->getCoordinate();
}
GeometryTypeId
MultiPoint::getGeometryTypeId() const
{
    return GEOS_MULTIPOINT;
}

const Point*
MultiPoint::getGeometryN(std::size_t i) const
{
    return static_cast<const Point*>(geometries[i].get());
}

} // namespace geos::geom
} // namespace geos
