/**********************************************************************
 * $Id: LengthIndexOfPoint.h 1938 2009-07-23 10:45:16Z novalis $
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
 **********************************************************************
 *
 * Last port: linearref/LengthIndexOfPoint.java rev. 1.10
 *
 **********************************************************************/

#ifndef GEOS_LINEARREF_LENGTHINDEXOFPOINT_H
#define GEOS_LINEARREF_LENGTHINDEXOFPOINT_H

#include <string>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineSegment.h>
#include <geos/linearref/LinearLocation.h>


namespace geos
{
namespace linearref   // geos::linearref
{

class LengthIndexOfPoint
{

private:
	geom::Geometry *linearGeom;

	double indexOfFromStart(geom::Coordinate& inputPt, double minIndex);

	double segmentNearestMeasure(geom::LineSegment *seg, geom::Coordinate& inputPt,
				     double segmentStartMeasure);
public:
	static double indexOf(geom::Geometry *linearGeom, geom::Coordinate& inputPt);

	static double indexOfAfter(geom::Geometry *linearGeom, geom::Coordinate& inputPt, double minIndex);

	LengthIndexOfPoint(geom::Geometry *linearGeom);

	/**
	 * Find the nearest location along a linear {@link Geometry} to a given point.
	 *
	 * @param inputPt the coordinate to locate
	 * @return the location of the nearest point
	 */
	double indexOf(geom::Coordinate& inputPt);

	/**
	 * Finds the nearest index along the linear {@link Geometry}
	 * to a given {@link Coordinate}
	 * after the specified minimum index.
	 * If possible the location returned will be strictly greater than the
	 * <code>minLocation</code>.
	 * If this is not possible, the
	 * value returned will equal <code>minLocation</code>.
	 * (An example where this is not possible is when
	 * minLocation = [end of line] ).
	 *
	 * @param inputPt the coordinate to locate
	 * @param minLocation the minimum location for the point location
	 * @return the location of the nearest point
	 */
	double indexOfAfter(geom::Coordinate& inputPt, double minIndex);

};
}
}
#endif
