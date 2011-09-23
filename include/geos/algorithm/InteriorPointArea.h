/**********************************************************************
 * $Id: InteriorPointArea.h 2556 2009-06-06 22:22:28Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_INTERIORPOINTAREA_H
#define GEOS_ALGORITHM_INTERIORPOINTAREA_H

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Geometry;
		class LineString;
		class GeometryFactory;
		class GeometryCollection;
	}
}


namespace geos {
namespace algorithm { // geos::algorithm

/** \brief
 * Computes a point in the interior of an area geometry.
 *
 * <h2>Algorithm</h2>
 * <ul>
 *   <li>Find the intersections between the geometry
 *       and the horizontal bisector of the area's envelope
 *   <li>Pick the midpoint of the largest intersection (the intersections
 *       will be lines and points)
 * </ul>
 *
 * <b>
 * Note: If a fixed precision model is used,
 * in some cases this method may return a point
 * which does not lie in the interior.
 * </b>
 */
class GEOS_DLL InteriorPointArea {

private:

	bool foundInterior;

	const geom::GeometryFactory *factory;

	geom::Coordinate interiorPoint;

	double maxWidth;

	void add(const geom::Geometry *geom);

	const geom::Geometry *widestGeometry(const geom::Geometry *geometry);

	const geom::Geometry *widestGeometry(const geom::GeometryCollection *gc);

	geom::LineString *horizontalBisector(const geom::Geometry *geometry);

public:

	InteriorPointArea(const geom::Geometry *g);

	~InteriorPointArea();

	bool getInteriorPoint(geom::Coordinate& ret) const;

	/** \brief
	 * Finds a reasonable point at which to label a Geometry.
	 *
	 * @param geometry the geometry to analyze
	 * @return the midpoint of the largest intersection between the geometry and
	 * a line halfway down its envelope
	 */
	void addPolygon(const geom::Geometry *geometry);

};

} // namespace geos::algorithm
} // namespace geos

#endif // GEOS_ALGORITHM_INTERIORPOINTAREA_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

