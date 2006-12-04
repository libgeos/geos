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


#ifndef GEOS_GEOMGRAPH_EDGENODINGVALIDATOR_H
#define GEOS_GEOMGRAPH_EDGENODINGVALIDATOR_H

#include <vector>

#include <geos/noding/NodingValidator.h> // for member

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class CoordinateSequence;
	}
	namespace noding {
		class SegmentString;
	}
	namespace geomgraph {
		class Edge;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

/**
 * Validates that a collection of SegmentStrings is correctly noded.
 * Throws an appropriate exception if an noding error is found.
 *
 * @version 1.4
 */
class EdgeNodingValidator {

private:
	std::vector<noding::SegmentString*>& toSegmentStrings(std::vector<Edge*>& edges);
	noding::NodingValidator nv;
	std::vector<noding::SegmentString*> segStr;
	std::vector<geom::CoordinateSequence*> newCoordSeq;

public:

	EdgeNodingValidator(std::vector<Edge*>& edges)
		:
		nv(toSegmentStrings(edges))
	{}

	~EdgeNodingValidator();

	void checkValid() { nv.checkValid(); }
};


} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeNodingValidator.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGENODINGVALIDATOR_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

