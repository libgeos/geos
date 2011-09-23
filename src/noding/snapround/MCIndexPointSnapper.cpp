/**********************************************************************
 * $Id: MCIndexPointSnapper.cpp 3240 2011-02-23 14:37:39Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/MCIndexPointSnapper.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/noding/snapround/MCIndexPointSnapper.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/noding/snapround/SimpleSnapRounder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/spatialIndex.h>
#include <geos/geom/Envelope.h>
#include <geos/index/chain/MonotoneChainSelectAction.h> 
#include <geos/index/chain/MonotoneChain.h> 
#include <geos/util.h>

#include <algorithm>

using namespace geos::index;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

class HotPixelSnapAction: public index::chain::MonotoneChainSelectAction {

public:

	HotPixelSnapAction(HotPixel& nHotPixel,
			SegmentString* nParentEdge,
			unsigned int nVertexIndex)
		:
		MonotoneChainSelectAction(),
		hotPixel(nHotPixel),
		parentEdge(nParentEdge),
		vertexIndex(nVertexIndex),
		isNodeAddedVar(false)
	{}

	bool isNodeAdded() const { return isNodeAddedVar; }

	void select(chain::MonotoneChain& mc, unsigned int startIndex)
	{
		// This is casting away 'constness'!
		NodedSegmentString& ss = *(static_cast<NodedSegmentString*>(mc.getContext()));

		// don't snap a vertex to itself
		if ( parentEdge ) {
			if (&ss == parentEdge && startIndex == vertexIndex) return;
		}

		//isNodeAddedVar = SimpleSnapRounder::addSnappedNode(hotPixel, ss, startIndex);

		isNodeAddedVar = hotPixel.addSnappedNode(ss, startIndex);
	}

	void select(const LineSegment& ls)
	{
		::geos::ignore_unused_variable_warning(ls);
	}

private:
	HotPixel& hotPixel;
	SegmentString* parentEdge;
	unsigned int vertexIndex;
	bool isNodeAddedVar;

    // Declare type as noncopyable
    HotPixelSnapAction(const HotPixelSnapAction& other);
    HotPixelSnapAction& operator=(const HotPixelSnapAction& rhs);
};

class MCIndexPointSnapperVisitor: public ItemVisitor {

public:
	MCIndexPointSnapperVisitor(const Envelope& nPixelEnv, HotPixelSnapAction& nAction)
		:
		pixelEnv(nPixelEnv),
		action(nAction)
	{}

	virtual ~MCIndexPointSnapperVisitor() {}

	void visitItem(void* item) {
		chain::MonotoneChain& testChain =
			*(static_cast<chain::MonotoneChain*>(item));
		testChain.select(pixelEnv, action);
	}

private:
	const Envelope& pixelEnv;
	chain::MonotoneChainSelectAction& action;

    // Declare type as noncopyable
    MCIndexPointSnapperVisitor(const MCIndexPointSnapperVisitor& other);
    MCIndexPointSnapperVisitor& operator=(const MCIndexPointSnapperVisitor& rhs);
};

/* public */
bool
MCIndexPointSnapper::snap(HotPixel& hotPixel,
		SegmentString* parentEdge,
		unsigned int vertexIndex)
{
	const Envelope& pixelEnv = hotPixel.getSafeEnvelope();
	HotPixelSnapAction hotPixelSnapAction(hotPixel, parentEdge, vertexIndex);
	MCIndexPointSnapperVisitor visitor(pixelEnv, hotPixelSnapAction);

	index.query(&pixelEnv, visitor);

	return hotPixelSnapAction.isNodeAdded();
}
 
} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/03/24 09:25:02  strk
 * Bugs #77 and #76: missing <algorithm>
 *
 * Revision 1.8  2006/03/22 18:12:32  strk
 * indexChain.h header split.
 *
 * Revision 1.7  2006/03/15 09:51:13  strk
 * streamlined headers usage
 *
 * Revision 1.6  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.5  2006/02/21 16:53:49  strk
 * MCIndexPointSnapper, MCIndexSnapRounder
 *
 * Revision 1.4  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.3  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.2  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
