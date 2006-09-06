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

#ifndef GEOS_OP_OVERLAY_FUZZYPOINTLOCATOR_H
#define GEOS_OP_OVERLAY_FUZZYPOINTLOCATOR_H

#include <geos/algorithm/PointLocator.h> // for composition

#include <vector>
#include <memory> // for auto_ptr

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
		class Coordinate;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay

/** \brief
 * Finds the location of a point relative to
 * a geometry using a tolerance value
 * to decide whether the point in on the boundary.
 */
class FuzzyPointLocator {

public:

	FuzzyPointLocator(const geom::Geometry& geom, double nTolerance);

	int getLocation(const geom::Coordinate& pt);

private:

	const geom::Geometry& g;

	double tolerance;

	algorithm::PointLocator ptLocator;

	std::auto_ptr<geom::Geometry> linework;

	std::auto_ptr<geom::Geometry> getLineWork(const geom::Geometry& geom);

};

} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_FUZZYPOINTLOCATOR_H

/**********************************************************************
 * $Log$
 **********************************************************************/

