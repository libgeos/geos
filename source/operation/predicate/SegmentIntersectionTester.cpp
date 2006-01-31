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
 * Last port: operation/predicate/SegmentIntersectionTester.java rev. 1.6
 * (JTS-1.7)
 *
 **********************************************************************/

#include <geos/opPredicate.h>
#include <geos/profiler.h>

namespace geos {
namespace operation {
namespace predicate {

bool
SegmentIntersectionTester::hasIntersectionWithLineStrings(
	const CoordinateSequence &seq,
	const LineString::ConstVect& lines)
{
	for (unsigned int i=0, n=lines.size(); i<n; ++i )
	{
		const LineString *line = lines[i];
		hasIntersection(seq, *(line->getCoordinatesRO()));
		if (hasIntersectionVar) break;
	}
	return hasIntersectionVar;
}

bool
SegmentIntersectionTester::hasIntersection(
	const CoordinateSequence &seq0, const CoordinateSequence &seq1)
{

	for (unsigned i=1, ni=seq0.getSize(); i<ni; ++i)
	{
		const Coordinate &pt00 = seq0.getAt(i - 1);
		const Coordinate &pt01 = seq0.getAt(i);
		for (unsigned j=1, nj=seq1.getSize(); j<nj; ++j)
		{
			const Coordinate &pt10 = seq1.getAt(j-1);
			const Coordinate &pt11 = seq1.getAt(j); 

			li.LineIntersector::computeIntersection(pt00, pt01,
				pt10, pt11);
			if (li.hasIntersection())
			{
				hasIntersectionVar = true;
				goto out_of_loop;
			}
		}
	}

	out_of_loop:

	return hasIntersectionVar;
}

} // namespace predicate
} // namespace operation
} // namespace geos



