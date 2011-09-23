/**********************************************************************
 * $Id: SimpleNoder.h 2556 2009-06-06 22:22:28Z strk $
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
 * Last port: noding/SimpleNoder.java rev. 1.7 (JTS-1.9)
 *
 **********************************************************************/

#ifndef GEOS_NODING_SIMPLENODER_H
#define GEOS_NODING_SIMPLENODER_H

#include <geos/export.h>

#include <vector>

#include <geos/inline.h>

#include <geos/noding/SinglePassNoder.h>
#include <geos/noding/NodedSegmentString.h> // for inlined (FIXME)

// Forward declarations
namespace geos {
	namespace noding {
		//class SegmentString;
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
class GEOS_DLL SimpleNoder: public SinglePassNoder {
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
		return NodedSegmentString::getNodedSubstrings(*nodedSegStrings);
	}
};

} // namespace geos.noding
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/noding/SimpleNoder.inl"
//#endif

#endif // GEOS_NODING_SIMPLENODER_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/15 09:51:49  strk
 * streamlined header usage
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

