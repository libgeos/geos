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

#include <geos/export.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <vector>

#include <geos/geom/util/PolygonalExtracter.h>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

void
PolygonalExtracter::getPolygonals(const Geometry& geom, std::vector<const Geometry*>& polys)
{
    getPolygonals(&geom, polys);
}

void
PolygonalExtracter::getPolygonals(const Geometry* geom, std::vector<const Geometry*>& polys)
{
   if (dynamic_cast<const Polygon*>(geom) != nullptr
         || dynamic_cast<const MultiPolygon*>(geom) != nullptr ) {
  		polys.push_back(geom);
  	}
  	else if (dynamic_cast<const GeometryCollection*>(geom) != nullptr) {
  	  for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
  	    getPolygonals(geom->getGeometryN(i), polys);
  	  }
  	}
}

}
}
}
