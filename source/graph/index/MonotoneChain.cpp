#include "../../headers/graphindex.h"

namespace geos {

MonotoneChain::MonotoneChain(MonotoneChainEdge *newMce,int newChainIndex) {
	mce=newMce;
	chainIndex=newChainIndex;
}

void MonotoneChain::computeIntersections(MonotoneChain *mc,SegmentIntersector *si){
	mce->computeIntersectsForChain(chainIndex,mc->mce,mc->chainIndex,si);
}

}

