/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_LOCATE_SIMPLEPOINTINAREALOCATOR_H
#define GEOS_ALGORITHM_LOCATE_SIMPLEPOINTINAREALOCATOR_H

#include <geos/algorithm/locate/PointOnGeometryLocator.h> // inherited

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
		class Coordinate;
		class Polygon;
	}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace locate { // geos::algorithm::locate

class SimplePointInAreaLocator : public PointOnGeometryLocator
{

public:

	static int locate(const geom::Coordinate& p,
			const geom::Geometry *geom);

	static bool containsPointInPolygon(const geom::Coordinate& p,
			const geom::Polygon *poly);

	SimplePointInAreaLocator( const geom::Geometry * g) 
	:	g( g)
	{ }

	int locate( const geom::Coordinate * p) 
	{
		return locate( *p, g);
	}

private:

	static bool containsPoint(const geom::Coordinate& p,
			const geom::Geometry *geom);

	const geom::Geometry * g;

};

} // geos::algorithm::locate
} // geos::algorithm
} // geos


#endif // GEOS_ALGORITHM_LOCATE_SIMPLEPOINTINAREALOCATOR_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

