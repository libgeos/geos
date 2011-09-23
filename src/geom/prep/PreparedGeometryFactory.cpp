/**********************************************************************
 * $Id: PreparedGeometryFactory.cpp 2418 2009-04-29 08:15:21Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/prep/PreparedGeometryFactory.java rev. 1.4 (JTS-1.10)
 *
 **********************************************************************/


#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/BasicPreparedGeometry.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/geom/prep/PreparedLineString.h>
#include <geos/geom/prep/PreparedPoint.h>
#include <geos/util/IllegalArgumentException.h>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep

const PreparedGeometry *
PreparedGeometryFactory::create( const geom::Geometry * g) const
{
    using geos::geom::GeometryTypeId;

    if (0 == g)
    {
        throw util::IllegalArgumentException("PreparedGeometry constructd with null Geometry object");
    }

	PreparedGeometry* pg = 0;

	switch ( g->getGeometryTypeId() )
	{
		case GEOS_MULTIPOINT:
		case GEOS_POINT:
			pg = new PreparedPoint( g);
			break;

		case GEOS_LINEARRING:
		case GEOS_LINESTRING:
		case GEOS_MULTILINESTRING:
			pg = new PreparedLineString( g);
			break;

		case GEOS_POLYGON:
		case GEOS_MULTIPOLYGON:
			pg = new PreparedPolygon( g);
			break;

		default:
			pg = new BasicPreparedGeometry( g);
	}
	return pg;
}

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos

/**********************************************************************
 * $Log$
 *
 **********************************************************************/
