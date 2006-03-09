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

#ifndef GEOS_ALGORITHM_SIRTREEPOINTINRING_H
#define GEOS_ALGORITHM_SIRTREEPOINTINRING_H

#include <vector>

// FIXME: to be changed to <geos/index/strtree/SIRtree.h>
#include <geos/indexStrtree.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class LineSegment;
		class LinearRing;
	}
}


namespace geos {
namespace algorithm { // geos::algorithm

class SIRtreePointInRing: public PointInRing {
private:
	geom::LinearRing *ring;
	index::strtree::SIRtree *sirTree;
	int crossings;  // number of segment/ray crossings
	void buildIndex();
	void testLineSegment(const geom::Coordinate& p,
			geom::LineSegment *seg);
public:
	SIRtreePointInRing(geom::LinearRing *newRing);
	bool isInside(const geom::Coordinate& pt);
};

} // namespace geos::algorithm
} // namespace geos


#endif // GEOS_ALGORITHM_SIRTREEPOINTINRING_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

