#include "indexChain.h"
#include "stdio.h"

MonotoneChainOverlapAction::MonotoneChainOverlapAction() {
	seg1=new LineSegment();
	seg2=new LineSegment();
}

MonotoneChainOverlapAction::~MonotoneChainOverlapAction() {
	delete seg1;
	delete seg2;
}


/**
 * This function can be overridden if the original chains are needed
 */
void MonotoneChainOverlapAction::overlap(indexMonotoneChain *mc1,int start1,indexMonotoneChain *mc2,int start2) {
	mc1->getLineSegment(start1,seg1);
	mc2->getLineSegment(start2,seg2);
	overlap(seg1,seg2);
}
