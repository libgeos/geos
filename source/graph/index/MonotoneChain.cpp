#include "graphindex.h"

MonotoneChain::MonotoneChain(MonotoneChainEdge *newMce,int newChainIndex,int newGeomIndex) {
	mce=newMce;
	chainIndex=newChainIndex;
	geomIndex=newGeomIndex;
}

void MonotoneChain::computeIntersections(MonotoneChain *mc,SegmentIntersector *si){
	mce->computeIntersectsForChain(chainIndex,mc->mce,mc->chainIndex,si);
}

int MonotoneChain::getGeomIndex(){
	return geomIndex;
}

