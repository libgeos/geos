/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/operation/distance/GeometryLocation.h>

#include <sstream>

using geos::geom::Geometry;
using geos::geom::CoordinateXY;

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance

/**
 * Constructs a GeometryLocation specifying a point on a geometry, as well as the
 * segment that the point is on (or INSIDE_AREA if the point is not on a segment).
 */
GeometryLocation::GeometryLocation(const Geometry* newComponent, std::size_t newSegIndex, const CoordinateXY& newPt)
{
    component = newComponent;
    segIndex = newSegIndex;
    inside_area = false;
    pt = newPt;
}

/**
 * Constructs a GeometryLocation specifying a point inside an area geometry.
 */
GeometryLocation::GeometryLocation(const Geometry* newComponent, const CoordinateXY& newPt)
{
    component = newComponent;
    inside_area = true;
    segIndex = (std::size_t) INSIDE_AREA;
    pt = newPt;
}

/**
 * Returns the geometry associated with this location.
 */
const Geometry*
GeometryLocation::getGeometryComponent()
{
    return component;
}
/**
 * Returns the segment index for this location. If the location is inside an
 * area, the index will have the value INSIDE_AREA;
 *
 * @return the segment index for the location, or INSIDE_AREA
 */
size_t
GeometryLocation::getSegmentIndex()
{
    return segIndex;
}
/**
 * Returns the location.
 */
CoordinateXY&
GeometryLocation::getCoordinate()
{
    return pt;
}

bool
GeometryLocation::isInsideArea()
{
    return inside_area;
}

std::string
GeometryLocation::toString()
{
    geos::io::WKTWriter writer;
    std::ostringstream ss;
    ss << component->getGeometryType();
    ss << "[";
    ss << std::to_string(segIndex);
    ss << "]-";
    ss << writer.toPoint(pt);
    return ss.str();
}

} // namespace geos.operation.distance
} // namespace geos.operation
} // namespace geos
