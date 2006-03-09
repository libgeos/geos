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

#ifndef GEOS_NODING_SIMPLENODER_H
#define GEOS_NODING_SIMPLENODER_H

#include <vector>

#include <geos/inline.h>

#include <geos/noding/SinglePassNoder.h>

// Forward declarations
namespace geos {
	namespace noding {
		class SegmentString;
	}
}

namespace geos {
namespace noding { // geos.noding


/** \brief
 * Nodes a set of {@link SegmentString}s by
 * performing a brute-force comparison of every segment to every other one.
 *
 * This has n^2 performance, so is too slow for use on large numbers
 * of segments.
 *
 * @version 1.7
 */
class SimpleNoder: public SinglePassNoder {
private:
	std::vector<SegmentString*>* nodedSegStrings;
	virtual void computeIntersects(SegmentString *e0, SegmentString *e1);

public:
	SimpleNoder(SegmentIntersector* nSegInt=NULL)
		:
		SinglePassNoder(nSegInt)
	{};

	void computeNodes(std::vector<SegmentString*>* inputSegmentStrings);

	std::vector<SegmentString*>* getNodedSubstrings() const {
		return SegmentString::getNodedSubstrings(*nodedSegStrings);
	}
};

} // namespace geos.noding
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/noding/SimpleNoder.inl"
//#endif

#endif // GEOS_NODING_SIMPLENODER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

