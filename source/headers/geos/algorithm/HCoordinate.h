/**********************************************************************
 * $Id$
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
 **********************************************************************
 *
 * Last port: algorithm/HCoordinate.java rev. 1.17 (JTS-1.7)
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_HCOORDINATE_H
#define GEOS_ALGORITHM_HCOORDINATE_H

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
}

namespace geos {
namespace algorithm { // geos::algorithm


/** \brief
 * Represents a homogeneous coordinate in a 2-D coordinate space.
 *
 * HCoordinate are used as a clean way
 * of computing intersections between line segments.
 */
class HCoordinate {
public:

	/** \brief
	 * Computes the (approximate) intersection point between two line segments
	 * using homogeneous coordinates.
	 * 
	 * Note that this algorithm is
	 * not numerically stable; i.e. it can produce intersection points which
	 * lie outside the envelope of the line segments themselves.  In order
	 * to increase the precision of the calculation input points should be normalized
	 * before passing them to this routine.
	 */
	static void intersection(const geom::Coordinate &p1, const geom::Coordinate &p2,
		const geom::Coordinate &q1, const geom::Coordinate &q2, geom::Coordinate &ret);

	double x,y,w;

	HCoordinate();

	HCoordinate(double _x, double _y, double _w);

	HCoordinate(const geom::Coordinate& p);

	HCoordinate(const HCoordinate &p1, const HCoordinate &p2);

	double getX() const;

	double getY() const;

	void getCoordinate(geom::Coordinate &ret) const;

};

} // namespace geos::algorithm
} // namespace geos

#endif // GEOS_ALGORITHM_HCOORDINATE_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/04 11:37:01  strk
 * Port information + initialization lists in ctors
 *
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

