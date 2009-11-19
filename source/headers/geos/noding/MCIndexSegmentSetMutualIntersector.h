/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 *
 **********************************************************************/

#ifndef GEOS_NODING_MCINDEXSEGMENTSETMUTUALINTERSECTOR_H
#define GEOS_NODING_MCINDEXSEGMENTSETMUTUALINTERSECTOR_H

#include <geos/noding/SegmentSetMutualIntersector.h> // inherited
#include <geos/index/chain/MonotoneChainOverlapAction.h> // inherited

namespace geos {
	namespace index {
		class SpatialIndex;

		namespace chain {
			class MonotoneChain;
		}
		namespace strtree {
			//class STRtree;
		}
	}
	namespace noding {
		class SegmentString;
		class SegmentIntersector;
	}
}

//using namespace geos::index::strtree;

namespace geos {
namespace noding { // geos::noding

/** \brief
 * Intersects two sets of {@link SegmentStrings} using a index based
 * on {@link MonotoneChain}s and a {@link SpatialIndex}.
 *
 * @version 1.7
 */
class MCIndexSegmentSetMutualIntersector : public SegmentSetMutualIntersector 
{
public:

	MCIndexSegmentSetMutualIntersector();

	~MCIndexSegmentSetMutualIntersector();

	std::vector<index::chain::MonotoneChain *>& getMonotoneChains() 
	{ 
		return monoChains; 
	}

	index::SpatialIndex* getIndex() 
	{ 
		return index; 
	}

	void setBaseSegments(SegmentString::ConstVect* segStrings);
  
	void process(SegmentString::ConstVect* segStrings);

	class SegmentOverlapAction : public index::chain::MonotoneChainOverlapAction
	{
	private:
		SegmentIntersector & si;

	public:

		SegmentOverlapAction(SegmentIntersector & si)
			:
			index::chain::MonotoneChainOverlapAction(),
			si(si)
		{}

		void overlap(index::chain::MonotoneChain& mc1, size_t start1,
		             index::chain::MonotoneChain& mc2, size_t start2);
	};

private:

	typedef std::vector<index::chain::MonotoneChain *> MonoChains;
	MonoChains monoChains;

	/*
	 * The {@link SpatialIndex} used should be something that supports
	 * envelope (range) queries efficiently (such as a {@link Quadtree}
	 * or {@link STRtree}.
	 */
	index::SpatialIndex * index;
	int indexCounter;
	int processCounter;
	// statistics
	int nOverlaps;
	
	// memory management helper
      typedef std::vector<std::vector<index::chain::MonotoneChain*>*> chainstore_mm_type;
      chainstore_mm_type chainStore;
      
	void addToIndex( SegmentString * segStr);

	void intersectChains();

	void addToMonoChains( SegmentString * segStr);

};

} // namespace geos::noding
} // namespace geos

#endif // GEOS_NODING_MCINDEXSEGMENTSETMUTUALINTERSECTOR_H
/**********************************************************************
 * $Log$
 **********************************************************************/

