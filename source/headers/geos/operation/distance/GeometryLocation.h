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

#ifndef GEOS_OP_DISTANCE_GEOMETRYLOCATION_H
#define GEOS_OP_DISTANCE_GEOMETRYLOCATION_H

#include <geos/geom/Coordinate.h> // for composition

// Forward declarations
namespace geos {
	namespace geom { 
		class Geometry;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace distance { // geos::operation::distance


/** \brief
 * Represents the location of a point on a Geometry.
 *
 * Maintains both the actual point location (which of course
 * may not be exact) as well as information about the component
 * and segment index where the point occurs.
 * Locations inside area Geometrys will not have an associated segment index,
 * so in this case the segment index will have the sentinel value of
 * INSIDE_AREA.
 */
class GeometryLocation {
private:
	const geom::Geometry *component;
	int segIndex;
	geom::Coordinate pt;
public:  
	/**
	 * Special value of segment-index for locations inside area geometries. These
	 * locations do not have an associated segment index.
	 */
	static const int INSIDE_AREA = -1;

	/**
	 * Constructs a GeometryLocation specifying a point on a geometry, as well as the 
	 * segment that the point is on (or INSIDE_AREA if the point is not on a segment).
	 */
	GeometryLocation(const geom::Geometry *newComponent,
			int newSegIndex, const geom::Coordinate &newPt);

	/**
	 * Constructs a GeometryLocation specifying a point inside an area geometry.
	 */  
	GeometryLocation(const geom::Geometry *newComponent, const geom::Coordinate &newPt);

	/**
	 * Returns the geometry associated with this location.
	 */
	const geom::Geometry* getGeometryComponent();

	/**
	 * Returns the segment index for this location. If the location is inside an
	 * area, the index will have the value INSIDE_AREA;
	 *
	 * @return the segment index for the location, or INSIDE_AREA
	 */
	int getSegmentIndex();

	/**
	 * Returns the location.
	 */
	geom::Coordinate& getCoordinate();

	/**
	 * Returns whether this GeometryLocation represents a point inside an area geometry.
	 */
	bool isInsideArea();
};

} // namespace geos::operation::distance
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_DISTANCE_GEOMETRYLOCATION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 17:55:01  strk
 * opDistance.h header split
 *
 **********************************************************************/

