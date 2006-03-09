/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_INTERSECTIONFINDERADDER_H
#define GEOS_NODING_INTERSECTIONFINDERADDER_H

#include <vector>
#include <iostream>

#include <geos/inline.h>

#include <geos/geom/Coordinate.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
	namespace noding {
		class SegmentString;
	}
	namespace algorithm {
		class LineIntersector;
	}
}

namespace geos {
namespace noding { // geos.noding

/** \brief
 * Finds proper and interior intersections in a set of SegmentStrings,
 * and adds them as nodes.
 *
 * Last port: noding/IntersectionFinderAdder.java rev. 1.2 (JTS-1.7)
 *
 */
class IntersectionFinderAdder: public SegmentIntersector {

private:
	algorithm::LineIntersector& li;
	std::vector<geom::Coordinate>& interiorIntersections;

public:

	/**
	 * Creates an intersection finder which finds all proper intersections
	 * and stores them in the provided Coordinate array
	 *
	 * @param li the LineIntersector to use
	 */
	IntersectionFinderAdder(algorithm::LineIntersector& newLi, std::vector<geom::Coordinate>& v)
		:
		li(newLi),
		interiorIntersections(v)
	{}

	/**
	 * This method is called by clients
	 * of the {@link SegmentIntersector} class to process
	 * intersections for two segments of the {@link SegmentStrings}
	 * being intersected.
	 * Note that some clients (such as {@link MonotoneChain}s) may
	 * optimize away this call for segment pairs which they have
	 * determined do not intersect
	 * (e.g. by an disjoint envelope test).
	 */
	void processIntersections(
		SegmentString* e0,  int segIndex0,
		SegmentString* e1,  int segIndex1);

	std::vector<geom::Coordinate>& getInteriorIntersections() {
		return interiorIntersections;
	}
	
};

} // namespace geos.noding
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/noding/IntersectionFinderAdder.inl"
//#endif

#endif // GEOS_NODING_INTERSECTIONFINDERADDER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

