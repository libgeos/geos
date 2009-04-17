/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2009  Sandro Santilli <strk@keybit.net>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: jtstest/testrunner/BufferResultMatcher.java rev rev 1.5 (JTS-1.10)
 *
 **********************************************************************/

#ifndef XMLTESTER_BUFFERRESULTMATCHER_H
#define XMLTESTER_BUFFERRESULTMATCHER_H

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
	}
}

namespace geos {
namespace xmltester {

class BufferResultMatcher
{
public:
	bool isBufferResultMatch(const geom::Geometry& actualBuffer,
	                         const geom::Geometry& expectedBuffer,
	                         double distance);

private:

	static double MAX_RELATIVE_AREA_DIFFERENCE;

	static double MAX_HAUSDORFF_DISTANCE_FACTOR;

	bool isSymDiffAreaInTolerance(const geom::Geometry& actualBuffer,
	                              const geom::Geometry& expectedBuffer);

	bool isBoundaryHausdorffDistanceInTolerance(
                        const geom::Geometry& actualBuffer,
	                const geom::Geometry& expectedBuffer,
	                double distance);
};

} // namespace geos::xmltester
} // namespace geos

#endif // XMLTESTER_BUFFERRESULTMATCHER_H
