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
 ***********************************************************************
 *
 * Last port: operation/overlay/validate/FuzzyPointLocator.java rev. 1.1 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_FUZZYPOINTLOCATOR_H
#define GEOS_OP_OVERLAY_FUZZYPOINTLOCATOR_H

#include <geos/algorithm/PointLocator.h> // for composition
#include <geos/geom/Geometry.h> // for auto_ptr visibility of dtor
#include <geos/geom/Location.h> // for Location::Value enum

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
namespace validate { // geos::operation::overlay::validate

/** \brief
 * Finds the most likely Location of a point relative to
 * the polygonal components of a geometry, using a tolerance value.
 *
 * If a point is not clearly in the Interior or Exterior,
 * it is considered to be on the Boundary.
 * In other words, if the point is within the tolerance of the Boundary,
 * it is considered to be on the Boundary; otherwise,
 * whether it is Interior or Exterior is determined directly.
 */
class FuzzyPointLocator {

public:

	FuzzyPointLocator(const geom::Geometry& geom, double nTolerance);

	geom::Location::Value getLocation(const geom::Coordinate& pt);

private:

	const geom::Geometry& g;

	double tolerance;

	algorithm::PointLocator ptLocator;

	std::auto_ptr<geom::Geometry> linework;

	// this function has been obsoleted
	std::auto_ptr<geom::Geometry> getLineWork(const geom::Geometry& geom);

	/// Extracts linework for polygonal components.
	//
	/// @param g the geometry from which to extract
	/// @return a lineal geometry containing the extracted linework
	std::auto_ptr<geom::Geometry> extractLineWork(const geom::Geometry& geom);

    // Declare type as noncopyable
    FuzzyPointLocator(const FuzzyPointLocator& other);
    FuzzyPointLocator& operator=(const FuzzyPointLocator& rhs);
};

} // namespace geos::operation::overlay::validate
} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_FUZZYPOINTLOCATOR_H

/**********************************************************************
 * $Log$
 **********************************************************************/

