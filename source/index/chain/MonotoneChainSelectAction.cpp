#include "../../headers/indexChain.h"
#include "stdio.h"

namespace geos {

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
}