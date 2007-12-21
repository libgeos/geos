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

#ifndef GEOS_GEOM_PREP_PREPAREDPOLYGONCONTAINSPROPERLY_H
#define GEOS_GEOM_PREP_PREPAREDPOLYGONCONTAINSPROPERLY_H

#include <geos/geom/prep/PreparedPolygonPredicate.h> // inherited

namespace geos {
	namespace geom { 
		class Geometry;

		namespace prep { 
			class PreparedPolygon;
		}
	}
}

namespace geos {
namespace geom { // geos::geom
namespace prep { // geos::geom::prep

class PreparedPolygonContainsProperly : public PreparedPolygonPredicate
{
private:
protected:
public:
	/**
	 * Computes the </tt>containsProperly</tt> predicate between a {@link PreparedPolygon}
	 * and a {@link Geometry}.
	 * 
	 * @param prep the prepared polygon
	 * @param geom a test geometry
	 * @return true if the polygon properly contains the geometry
	 */
	static 
	bool 
	containsProperly( const PreparedPolygon * const prep, const geom::Geometry * geom)
	{
		PreparedPolygonContainsProperly polyInt( prep);
		return polyInt.containsProperly( geom);
	}

	/**
	 * Creates an instance of this operation.
	 * 
	 * @param prepPoly the PreparedPolygon to evaluate
	 */
	PreparedPolygonContainsProperly( const PreparedPolygon * const prep)
	:	PreparedPolygonPredicate( prep)
	{ }
	
	/**
	 * Tests whether this PreparedPolygon containsProperly a given geometry.
	 * 
	 * @param geom the test geometry
	 * @return true if the test geometry is contained properly
	 */
	bool 
	containsProperly( const geom::Geometry * geom);

};

} // geos::geom::prep
} // geos::geom
} // geos

#endif // GEOS_GEOM_PREP_PREPAREDPOLYGONCONTAINSPROPERLY_H
/**********************************************************************
 * $Log$
 **********************************************************************/

