/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/relateng/LineStringExtracter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/constants.h>
#include <sstream>


using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public static */
void
LineStringExtracter::getLines(const Geometry* geom, std::vector<const LineString*>& lines)
{
    if (geom->getGeometryTypeId() == geom::GEOS_LINESTRING) {
        lines.push_back(static_cast<const LineString*>(geom));
    }
    else if (geom->isCollection()) {
        LineStringExtracter lse(lines);
        geom->apply_ro(&lse);
    }
    // skip non-LineString elemental geometries

    return;
}


/* public static */
std::vector<const LineString*>
LineStringExtracter::getLines(const Geometry* geom)
{
    std::vector<const LineString*> lines;
    getLines(geom, lines);
    return lines;
}


/* public static */
// std::unique_ptr<Geometry>
// LineStringExtracter::getGeometry(const Geometry* geom)
// {
//     std::vector<const LineString*> lines;
//     getLines(geom, lines);
//     return geom->getFactory()->buildGeometry(lines);
// }


/* public */
void
LineStringExtracter::filter_ro(const Geometry* geom)
{
    if (geom->getGeometryTypeId() == geom::GEOS_LINESTRING)
        comps.push_back(static_cast<const LineString*>(geom));
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




