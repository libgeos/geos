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
 * Last port: operation/overlay/OverlayResultValidator.java rev. 0
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_OVERLAYRESULTVALIDATOR_H
#define GEOS_OP_OVERLAY_OVERLAYRESULTVALIDATOR_H

#include <geos/operation/overlay/OverlayOp.h> // for OpCode enum
#include <geos/operation/overlay/FuzzyPointLocator.h> // for OpCode enum
#include <geos/geom/Location.h> // for Location::Value type

#include <vector>

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
 * Validates that the result of an overlay operation is
 * geometrically correct within a given tolerance.
 *
 * Uses fuzzy point location, and is only useful where the inputs are polygonal.
 * This is a heuristic test, and may return incorrect results.
 * It should never return a false negative result, however
 * (I.e. reporting a valid result as invalid.)
 * 
 * Probably only works for polygonal geometry inputs.
 *
 * @see OverlayOp
 */
class OverlayResultValidator {

public:

	static bool isValid(
			const geom::Geometry& geom0,
			const geom::Geometry& geom1,
			OverlayOp::OpCode opCode,
			const geom::Geometry& result);

	OverlayResultValidator(
			const geom::Geometry& geom0,
			const geom::Geometry& geom1,
			const geom::Geometry& result);

	bool isValid(OverlayOp::OpCode opCode);

	geom::Coordinate& getInvalidLocation() {
		return invalidLocation;
	}

private:

	const geom::Geometry& g0;

	const geom::Geometry& g1;

	const geom::Geometry& gres;

	FuzzyPointLocator fpl0;

	FuzzyPointLocator fpl1;

	FuzzyPointLocator fplres;

	geom::Coordinate invalidLocation;

	static double _TOLERANCE; // 0.000001
	
	std::vector<geom::Coordinate> testCoords;

	void addTestPts(const geom::Geometry& g);

	bool testValid(OverlayOp::OpCode overlayOp);

	bool testValid(OverlayOp::OpCode overlayOp, const geom::Coordinate& pt);

	bool isValidResult(OverlayOp::OpCode overlayOp,
			std::vector<geom::Location::Value>& location);
};

} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_OVERLAYRESULTVALIDATOR_H

/**********************************************************************
 * $Log$
 **********************************************************************/

