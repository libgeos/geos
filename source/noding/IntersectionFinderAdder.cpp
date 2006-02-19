/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
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
 * Last port: noding/IntersectionFinderAdder.java rev. 1.2 (JTS-1.7)
 *
 **********************************************************************/

#include "geos/noding.h"

namespace geos {
namespace noding { // geos.noding

void
IntersectionFinderAdder::processIntersections(
		SegmentString* e0,  int segIndex0,
		SegmentString* e1,  int segIndex1)
{
	// don't bother intersecting a segment with itself
	if (e0 == e1 && segIndex0 == segIndex1) return;

	const Coordinate& p00 = e0->getCoordinate(segIndex0);
	const Coordinate& p01 = e0->getCoordinate(segIndex0 + 1);
	const Coordinate& p10 = e1->getCoordinate(segIndex1);
	const Coordinate& p11 = e1->getCoordinate(segIndex1 + 1);

	li.computeIntersection(p00, p01, p10, p11);
//if (li.hasIntersection() && li.isProper()) Debug.println(li);

	if (li.hasIntersection())
	{
		if (li.isInteriorIntersection())
		{
			for (int intIndex=0, n=li.getIntersectionNum(); intIndex<n; intIndex++)
			{
				interiorIntersections.push_back(li.getIntersection(intIndex));
			}
        		e0->addIntersections(&li, segIndex0, 0);
			e1->addIntersections(&li, segIndex1, 1);
		}
	}
}

} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
