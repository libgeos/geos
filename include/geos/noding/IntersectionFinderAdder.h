/**********************************************************************
 * $Id: IntersectionFinderAdder.h 2778 2009-12-03 19:44:00Z mloskot $
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
 **********************************************************************
 *
 * Last port: noding/IntersectionFinderAdder.java rev. 1.5 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_NODING_INTERSECTIONFINDERADDER_H
#define GEOS_NODING_INTERSECTIONFINDERADDER_H

#include <geos/export.h>

#include <vector>
#include <iostream>

#include <geos/inline.h>

#include <geos/geom/Coordinate.h> // for use in vector
#include <geos/noding/SegmentIntersector.h> // for inheritance

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
 */
class GEOS_DLL IntersectionFinderAdder: public SegmentIntersector {

public:

	/**
	 * Creates an intersection finder which finds all proper intersections
	 * and stores them in the provided Coordinate array
	 *
	 * @param li the LineIntersector to use
	 * @param v  the Vector to push interior intersections to
	 */
	IntersectionFinderAdder(algorithm::LineIntersector& newLi,
			std::vector<geom::Coordinate>& v)
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
	
	/**
	 * Always process all intersections
	 *
	 * @return false always
	 */
	virtual bool isDone() const {
		return false;
	}

private:
	algorithm::LineIntersector& li;
	std::vector<geom::Coordinate>& interiorIntersections;
     
    // Declare type as noncopyable
    IntersectionFinderAdder(const IntersectionFinderAdder& other);
    IntersectionFinderAdder& operator=(const IntersectionFinderAdder& rhs);
};

} // namespace geos.noding
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/noding/IntersectionFinderAdder.inl"
//#endif

#endif // GEOS_NODING_INTERSECTIONFINDERADDER_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/05/03 15:06:30  strk
 * Better doxygen comment
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

