/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/SingleInteriorIntersectionFinder.java rev. 2019-01-28
 *
 **********************************************************************/

#include <geos/noding/SingleInteriorIntersectionFinder.h>
#include <geos/noding/SegmentString.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>

using namespace geos::geom;


namespace { // anonymous

	/* private in JTS */
	bool isEndInteriorIntersection(
	        const Coordinate& p0, bool isEnd0,
	        const Coordinate& p1, bool isEnd1)
	{
		if (isEnd0 && isEnd1) return false;
		if (p0.equals2D(p1)) {
			return true;
		}
		return false;
	}

	/* public in JTS */
	bool isEndInteriorIntersection(
	        const Coordinate& p00, const Coordinate& p01,
	        const Coordinate& p10, const Coordinate& p11,
	        bool isEnd00, bool isEnd01,
	        bool isEnd10, bool isEnd11)
	{
		if (isEndInteriorIntersection(p00, isEnd00, p10, isEnd10)) return true;
		if (isEndInteriorIntersection(p00, isEnd00, p11, isEnd11)) return true;
		if (isEndInteriorIntersection(p01, isEnd01, p10, isEnd10)) return true;
		if (isEndInteriorIntersection(p01, isEnd01, p11, isEnd11)) return true;
		return false;
	}

} // anonymous namespace


namespace geos {
namespace noding { // geos.noding

/*public (override) */
void
SingleInteriorIntersectionFinder::processIntersections(
	SegmentString* e0,  int segIndex0,
	SegmentString* e1,  int segIndex1)
{
	using geos::geom::Coordinate;

	// short-circuit if intersection already found
	if (hasIntersection())
		return;

	// don't bother intersecting a segment with itself
	if (e0 == e1 && segIndex0 == segIndex1) return;

	const Coordinate& p00 = e0->getCoordinate(segIndex0);
	const Coordinate& p01 = e0->getCoordinate(segIndex0 + 1);
	const Coordinate& p10 = e1->getCoordinate(segIndex1);
	const Coordinate& p11 = e1->getCoordinate(segIndex1 + 1);

	bool isEnd00 = segIndex0 == 0;
	bool isEnd01 = segIndex0 + 2 == static_cast<int>(e0->size());
	bool isEnd10 = segIndex1 == 0;
	bool isEnd11 = segIndex1 + 2 == static_cast<int>(e1->size());

	li.computeIntersection(p00, p01, p10, p11);
//if (li.hasIntersection() && li.isProper()) Debug.println(li);
	bool isProperInteriorInt = li.hasIntersection() &&
	                           li.isInteriorIntersection();
	bool isEndInteriorInt = (e0 != e1) && isEndInteriorIntersection(
	         p00, p01, p10, p11, isEnd00, isEnd01, isEnd10, isEnd11);

	if (isProperInteriorInt || isEndInteriorInt) {
		// found an intersection!
		intSegments.resize(4);
		intSegments[0] = p00;
		intSegments[1] = p01;
		intSegments[2] = p10;
		intSegments[3] = p11;
		interiorIntersection = li.getIntersection(0);
	}

}


} // namespace geos.noding
} // namespace geos
