#include "indexChain.h"
#include "stdio.h"

MonotoneChainSelectAction::MonotoneChainSelectAction() {
	seg=new LineSegment();
}

MonotoneChainSelectAction::~MonotoneChainSelectAction() {
	delete seg;
}

void MonotoneChainSelectAction::select(indexMonotoneChain *mc,int start) {
	mc->getLineSegment(start,seg);
	select(seg);
}
