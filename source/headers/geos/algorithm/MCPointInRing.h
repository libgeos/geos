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

#ifndef GEOS_ALGORITHM_MCPOINTINRING_H
#define GEOS_ALGORITHM_MCPOINTINRING_H

#include <vector>

// FIXME: to be changed to geos/index/chain/MonotoneChainSelectAction
#include <geos/indexChain.h>

#include <geos/geom/Coordinate.h>
#include <geos/algorithm/PointInRing.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class LineSegment;
		class LinearRing;
		class CoordinateSequence;
		class CoordinateSequence;
	}
	namespace index {
		namespace bintree {
			class Bintree;
			class Interval;
		}
		namespace chain {
			// when indexChain include is dropped
			// we'll need this
			//class MonotoneChain;
		}
	}
}

namespace geos {
namespace algorithm { // geos::algorithm

class MCPointInRing: public PointInRing {
public:
	MCPointInRing(geom::LinearRing *newRing);
	virtual ~MCPointInRing();
	bool isInside(const geom::Coordinate& pt);
	void testLineSegment(geom::Coordinate& p, geom::LineSegment *seg);

	class MCSelecter: public index::chain::MonotoneChainSelectAction {
	using MonotoneChainSelectAction::select;
	private:
		geom::Coordinate p;
		MCPointInRing *parent;
	public:
		MCSelecter(const geom::Coordinate& newP, MCPointInRing *prt);
		void select(geom::LineSegment *ls);
	};

private:
	geom::LinearRing *ring;
	index::bintree::Interval *interval;
	geom::CoordinateSequence *pts;
	index::bintree::Bintree *tree;
	int crossings;  // number of segment/ray crossings
	void buildIndex();
	void testMonotoneChain(geom::Envelope *rayEnv,
			MCSelecter *mcSelecter,
			index::chain::MonotoneChain *mc);
};

} // namespace geos::algorithm
} // namespace geos

#endif // GEOS_ALGORITHM_MCPOINTINRING_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/22 16:01:33  strk
 * indexBintree.h header split, classes renamed to match JTS
 *
 * Revision 1.1  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

