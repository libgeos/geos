/**********************************************************************
 * $Id
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
 *
 **********************************************************************/

#ifndef GEOS_GEOM_PREP_PREPAREDPOLYGONPREDICATE_H
#define GEOS_GEOM_PREP_PREPAREDPOLYGONPREDICATE_H

#include <geos/geom/Coordinate.h>

// forward declarations
namespace geos {
	namespace algorithm {
		namespace locate {
			class PointOnGeometryLocator;
		}
	}
	namespace geom { 
		class Geometry;

		namespace prep { 
			class PreparedPolygon;
		}
	}
	namespace noding {
		class FastSegmentSetIntersectionFinder;
	}
}


namespace geos {
namespace geom { // geos::geom
namespace prep { // geos::geom::prep

/**
 * \brief
 * A base class for predicate operations on {@link PreparedPolygon}s.
 * 
 * @author mbdavis
 *
 */
class PreparedPolygonPredicate 
{
private:
protected:
	const PreparedPolygon * const prepPoly;

	/**
	 * Tests whether all components of the test Geometry 
	 * are contained in the target geometry.
	 * Handles both linear and point components.
	 * 
	 * @param geom a geometry to test
	 * @return true if all componenta of the argument are contained in the target geometry
	 */
	bool isAllTestComponentsInTargetArea( const geom::Geometry * testGeom);

	/**
	 * Tests whether all components of the test Geometry 
	 * are contained in the interior of the target geometry.
	 * Handles both linear and point components.
	 * 
	 * @param geom a geometry to test
	 * @return true if all componenta of the argument are contained in the target geometry interior
	 */
	bool isAllTestComponentsInTargetInterior( const geom::Geometry * testGeom);

	/**
	 * Tests whether any component of the test Geometry intersects
	 * the area of the target geometry.
	 * Handles test geometries with both linear and point components.
	 * 
	 * @param geom a geometry to test
	 * @return true if any component of the argument intersects the prepared geometry
	 */
	bool isAnyTestComponentInTargetArea( const geom::Geometry * testGeom);

	/**
	 * Tests whether any component of the test Geometry intersects
	 * the interior of the target geometry.
	 * Handles test geometries with both linear and point components.
	 * 
	 * @param geom a geometry to test
	 * @return true if any component of the argument intersects the prepared area geometry interior
	 */
	bool isAnyTestComponentInTargetInterior( const geom::Geometry * testGeom);

	/**
	 * Tests whether any component of the target geometry 
	 * intersects the area of the test geometry 
	 * 
	 * @param geom the test geometry
	 * @param repPts the representative points of the target geometry
	 * @return true if any component intersects the areal test geometry
	 */
	bool isAnyTargetComponentInTestArea( const geom::Geometry * testGeom, const geom::Coordinate::ConstVect * targetRepPts);

public:
	/**
	 * Creates an instance of this operation.
	 * 
	 * @param prepPoly the PreparedPolygon to evaluate
	 */
	PreparedPolygonPredicate( const PreparedPolygon * const prepPoly)
	:	prepPoly( prepPoly)
	{ }

	~PreparedPolygonPredicate()
	{ }

};

} // namespace geos::geom::prep
} // namespace geos::geom
} // namespace geos

#endif // GEOS_GEOM_PREP_PREPAREDPOLYGONPREDICATE_H
/**********************************************************************
 * $Log$
 **********************************************************************/

