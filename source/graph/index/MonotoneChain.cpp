/*
* $Log$
* Revision 1.7  2003/10/15 15:30:32  strk
* Declared a SweepLineEventOBJ from which MonotoneChain and SweepLineSegment
* derive to abstract SweepLineEvent object previously done on void * pointers.
* No more compiler warnings...
*
*/
#include "../../headers/graphindex.h"

namespace geos {

MonotoneChain::MonotoneChain(MonotoneChainEdge *newMce,int newChainIndex) {
	mce=newMce;
	chainIndex=newChainIndex;
}

MonotoneChain::~MonotoneChain() {
}

void MonotoneChain::computeIntersections(MonotoneChain *mc,SegmentIntersector *si){
	mce->computeIntersectsForChain(chainIndex,mc->mce,mc->chainIndex,si);
}

}

