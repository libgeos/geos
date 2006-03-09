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


#ifndef GEOS_GEOMGRAPH_QUADRANT_H
#define GEOS_GEOMGRAPH_QUADRANT_H

#include <string>

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

/**
 * Utility functions for working with quadrants, which are numbered as follows:
 * <pre>
 * 1 | 0
 * --+--
 * 2 | 3
 * <pre>
 *
 */
class Quadrant {
public:
	/**
	 * Returns the quadrant of a directed line segment
	 * (specified as x and y displacements, which cannot both be 0).
	 */
	static int quadrant(double dx, double dy);

	/**
	 * Returns the quadrant of a directed line segment from p0 to p1.
	 */
	static int quadrant(const geom::Coordinate& p0, const geom::Coordinate& p1);

	/**
	 * Returns true if the quadrants are 1 and 3, or 2 and 4
	 */
	static bool isOpposite(int quad1, int quad2);

	/* 
	 * Returns the right-hand quadrant of the halfplane defined by
	 * the two quadrants,
	 * or -1 if the quadrants are opposite, or the quadrant if they
	 * are identical.
	 */
	static int commonHalfPlane(int quad1, int quad2);

	/**
	 * Returns whether the given quadrant lies within the given halfplane
	 * (specified by its right-hand quadrant).
	 */
	static bool isInHalfPlane(int quad, int halfPlane);

	/**
	 * Returns true if the given quadrant is 0 or 1.
	 */
	static bool isNorthern(int quad);
};


} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/Quadrant.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_QUADRANT_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

