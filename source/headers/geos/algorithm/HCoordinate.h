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


/// Represents a homogeneous coordinate for 2-D coordinates.
class HCoordinate {
public:

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
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

