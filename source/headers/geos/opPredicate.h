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
 **********************************************************************/

#ifndef GEOS_OPPREDICATE_H
#define GEOS_OPPREDICATE_H

#include <memory>
#include <vector>
#include <geos/platform.h>
#include <geos/operation.h>
#include <geos/opOverlay.h>
#include <geos/geomgraph.h>
#include <geos/noding.h>
#include <geos/geom.h>
#include <geos/geosAlgorithm.h>

namespace geos {
namespace operation {
namespace predicate {

/*
 * Tests if any line segments in two sets of CoordinateSequences intersect.
 * Optimized for small geometry size.
 * Short-circuited to return as soon an intersection is found.
 *
 * Last port: operation/predicate/SegmentIntersectionTester.java rev. 1.6
 * (JTS-1.7)
 *
 */

class SegmentIntersectionTester {

private:

	/// \brief
	/// For purposes of intersection testing,
	/// don't need to set precision model
	///
	LineIntersector li; // Robust

	bool hasIntersectionVar;

public:

	SegmentIntersectionTester(): hasIntersectionVar(false) {}

	bool hasIntersectionWithLineStrings(const CoordinateSequence &seq,
		const LineString::ConstVect& lines);

	bool hasIntersection(const CoordinateSequence &seq0,
		const CoordinateSequence &seq1);


};

/**
 * Optimized implementation of spatial predicate "intersects"
 * for cases where the first Geometry is a rectangle.
 * 
 * As a further optimization,
 * this class can be used directly to test many geometries against a single
 * rectangle.
 *
 * Last port: operation/predicate/RectangleIntersects.java rev. 1.3 (JTS-1.7)
 */
class RectangleIntersects {

private:

	const Polygon &rectangle;
 	const Envelope &rectEnv;


public:

	/**
	 * Crossover size at which brute-force intersection scanning
	 * is slower than indexed intersection detection.
	 * Must be determined empirically.  Should err on the
	 * safe side by making value smaller rather than larger.
	 */
	static const int MAXIMUM_SCAN_SEGMENT_COUNT;

	/*
	 * Create a new intersects computer for a rectangle.
	 *
	 * @param newRect a rectangular geometry
	 */
	RectangleIntersects(const Polygon &newRect):
		rectangle(newRect),
		rectEnv(*(newRect.getEnvelopeInternal())) {}

	bool intersects(const Geometry& geom);

	static bool intersects(const Polygon &rectangle, const Geometry &b)
	{
		RectangleIntersects rp(rectangle); 
		return rp.intersects(b);
	}

};

/**
 * Optimized implementation of spatial predicate "contains"
 * for cases where the first Geometry is a rectangle.
 * 
 * As a further optimization,
 * this class can be used directly to test many geometries against a single
 * rectangle.
 *
 * Last port: operation/predicate/RectangleContains.java rev 1.1 (JTS-1.7)
 */
class RectangleContains {

private:

	const Polygon& rectangle;
	const Envelope& rectEnv;

	bool isContainedInBoundary(const Geometry& geom);
	bool isPointContainedInBoundary(const Point& geom);
	bool isPointContainedInBoundary(const Coordinate &coord);
	bool isLineStringContainedInBoundary(const LineString &line);
	bool isLineSegmentContainedInBoundary(const Coordinate& p0,
			const Coordinate& p1);

public:

	static bool contains(const Polygon& rect, const Geometry& b)
	{
		RectangleContains rc(rect);
		return rc.contains(b);
	}

	/**
	 * Create a new contains computer for two geometries.
	 *
	 * @param rectangle a rectangular geometry
	 */
	RectangleContains(const Polygon& rect)
		:
		rectangle(rect),
		rectEnv(*(rect.getEnvelopeInternal()))
		{}

	bool contains(const Geometry& geom);


};


} // namespace predicate
} // namespace operation
} // namespace geos

#endif // ifndef GEOS_OPPREDICATE_H
