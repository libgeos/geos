/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/predicate/RectangleContains.java rev 1.1 (JTS-1.7)
 *
 **********************************************************************/

#ifndef GEOS_OP_PREDICATE_RECTANGLECONTAINS_H
#define GEOS_OP_PREDICATE_RECTANGLECONTAINS_H

#include <geos/export.h>

#include <geos/geom/Polygon.h> // for inlines

// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
		class Geometry;
		class Point;
		class Coordinate;
		class LineString;
		//class Polygon;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace predicate { // geos::operation::predicate

/** \brief
 * Optimized implementation of spatial predicate "contains"
 * for cases where the first Geometry is a rectangle.
 * 
 * As a further optimization,
 * this class can be used directly to test many geometries against a single
 * rectangle.
 *
 */
class GEOS_DLL RectangleContains {

private:

	const geom::Polygon& rectangle;
	const geom::Envelope& rectEnv;

	bool isContainedInBoundary(const geom::Geometry& geom);
	bool isPointContainedInBoundary(const geom::Point& geom);
	bool isPointContainedInBoundary(const geom::Coordinate &coord);
	bool isLineStringContainedInBoundary(const geom::LineString &line);
	bool isLineSegmentContainedInBoundary(const geom::Coordinate& p0,
			const geom::Coordinate& p1);

public:

	static bool contains(const geom::Polygon& rect, const geom::Geometry& b)
	{
		RectangleContains rc(rect);
		return rc.contains(b);
	}

	/**
	 * Create a new contains computer for two geometries.
	 *
	 * @param rect a rectangular geometry
	 */
	RectangleContains(const geom::Polygon& rect)
		:
		rectangle(rect),
		rectEnv(*(rect.getEnvelopeInternal()))
		{}

	bool contains(const geom::Geometry& geom);


};



} // namespace geos::operation::predicate
} // namespace geos::operation
} // namespace geos

#endif // ifndef GEOS_OP_PREDICATE_RECTANGLECONTAINS_H
