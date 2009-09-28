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
 * Last port: original work
 *
 **********************************************************************/

#include "SingleSidedBufferResultMatcher.h"

#include <geos/geom/Geometry.h>
#include <geos/geom/BinaryOp.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>

#include <cmath>

namespace geos {
namespace xmltester {

double SingleSidedBufferResultMatcher::MIN_DISTANCE_TOLERANCE = 1.0e-8;
double SingleSidedBufferResultMatcher::MAX_HAUSDORFF_DISTANCE_FACTOR = 100;

bool
SingleSidedBufferResultMatcher::isBufferResultMatch(const geom::Geometry& actualBuffer,
	                         const geom::Geometry& expectedBuffer,
	                         double distance)
{
	if (actualBuffer.isEmpty() && expectedBuffer.isEmpty())
		return true;

	if (! isBoundaryHausdorffDistanceInTolerance(actualBuffer,
	           expectedBuffer, distance))
	{
std::cerr << "isBoundaryHasudorffDistanceInTolerance failed" << std::endl;
		return false;
	}

	return true;
}

bool
SingleSidedBufferResultMatcher::isBoundaryHausdorffDistanceInTolerance(
	const geom::Geometry& actualBuffer,
	const geom::Geometry& expectedBuffer, double distance)
{
	typedef std::auto_ptr<geom::Geometry> GeomPtr;

	using geos::algorithm::distance::DiscreteHausdorffDistance;

	GeomPtr actualBdy ( actualBuffer.clone() );
	GeomPtr expectedBdy ( expectedBuffer.clone() );

	DiscreteHausdorffDistance haus(*actualBdy, *expectedBdy);
	haus.setDensifyFraction(0.25);

	double maxDistanceFound = haus.orientedDistance();

	double expectedDistanceTol = fabs(distance) / MAX_HAUSDORFF_DISTANCE_FACTOR;
	if (expectedDistanceTol < MIN_DISTANCE_TOLERANCE)
	{
		expectedDistanceTol = MIN_DISTANCE_TOLERANCE;
	}

	if (maxDistanceFound > expectedDistanceTol)
	{
std::cerr << "maxDistanceFound: " << maxDistanceFound << " tolerated " << expectedDistanceTol << std::endl;
		return false;
	}

	return true;
}

} // namespace geos::xmltester
} // namespace geos
