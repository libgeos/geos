#include "../../headers/indexChain.h"
#include "stdio.h"
#include "../../headers/util.h"

namespace geos {

indexMonotoneChain::indexMonotoneChain(CoordinateList *newPts,int nstart,int nend) {
	env=NULL;
	// these envelopes are created once and reused
	env1=new Envelope();
	env2=new Envelope();
	pts=newPts;
	start=nstart;
	end=nend;
}

indexMonotoneChain::~indexMonotoneChain() {
	delete env;
	delete env1;
	delete env2;
}

Envelope* indexMonotoneChain::getEnvelope() {
	if (env==NULL) {
		Coordinate& p0=pts->getAt(start);
		Coordinate& p1=pts->getAt(end);
		env=new Envelope(p0,p1);
	}
	return env;
}

int indexMonotoneChain::getStartIndex(){
	return start;
}
int indexMonotoneChain::getEndIndex(){
	return end;
}

void indexMonotoneChain::getLineSegment(int index,LineSegment *ls) {
	ls->p0=pts->getAt(index);
	ls->p1=pts->getAt(index+1);
}
/**
* Return the subsequence of coordinates forming this chain.
* Allocates a new array to hold the Coordinates
*/
CoordinateList* indexMonotoneChain::getCoordinates() {
	CoordinateList *coord=CoordinateListFactory::internalFactory->createCoordinateList();
	for(int i=start;i<=end;i++) {
		coord->add(pts->getAt(i));
	}
	return coord;
}

/**
* Determine all the line segments in the chain whose envelopes overlap
* the searchEnvelope,and process them
*/
void indexMonotoneChain::select(Envelope *searchEnv,MonotoneChainSelectAction *mcs) {
	computeSelect(searchEnv,start,end,mcs);
}

void indexMonotoneChain::computeSelect(Envelope *searchEnv,int start0,int end0,MonotoneChainSelectAction *mcs ) {
	Coordinate& p0=pts->getAt(start0);
	Coordinate& p1=pts->getAt(end0);
	env1->init(p0,p1);
	//Debug.println("trying:"+p0+p1+" [ "+start0+","+end0+" ]");
	// terminating condition for the recursion
	if(end0-start0==1) {
		//Debug.println("computeSelect:"+p0+p1);
		mcs->select(this,start0);
		return;
	}
	// nothing to do if the envelopes don't overlap
	if (!searchEnv->intersects(env1))
		return;
	// the chains overlap,so split each in half and iterate (binary search)
	int mid=(start0+end0)/2;
	// Assert: mid != start or end (since we checked above for end-start <= 1)
	// check terminating conditions before recursing
	if (start0<mid) {
		computeSelect(searchEnv,start0,mid,mcs);
	}
	if (mid<end0) {
		computeSelect(searchEnv,mid,end0,mcs);
	}
}

void indexMonotoneChain::computeOverlaps(indexMonotoneChain *mc,MonotoneChainOverlapAction *mco) {
	computeOverlaps(start,end,mc,mc->start,mc->end,mco);
}

void indexMonotoneChain::computeOverlaps(int start0,int end0,indexMonotoneChain *mc,int start1,int end1,MonotoneChainOverlapAction *mco){
	Coordinate& p00=pts->getAt(start0);
	Coordinate& p01=pts->getAt(end0);
	Coordinate& p10=mc->pts->getAt(start1);
	Coordinate& p11=mc->pts->getAt(end1);
	//Debug.println("computeIntersectsForChain:"+p00+p01+p10+p11);
	// terminating condition for the recursion
	if (end0-start0==1 && end1-start1==1) {
		mco->overlap(this,start0,mc,start1);
		return;
	}
	// nothing to do if the envelopes of these chains don't overlap
	env1->init(p00,p01);
	env2->init(p10,p11);
	if (!env1->intersects(env2)) return;
	// the chains overlap,so split each in half and iterate (binary search)
	int mid0=(start0+end0)/2;
	int mid1=(start1+end1)/2;
	// Assert: mid != start or end (since we checked above for end-start <= 1)
	// check terminating conditions before recursing
	if (start0<mid0) {
		if (start1<mid1)
			computeOverlaps(start0,mid0,mc,start1,mid1,mco);
		if (mid1<end1)
			computeOverlaps(start0,mid0,mc,mid1,end1,mco);
	}
	if (mid0<end0) {
		if (start1<mid1)
			computeOverlaps(mid0,end0,mc,start1,mid1,mco);
		if (mid1<end1)
			computeOverlaps(mid0,end0,mc,mid1,end1,mco);
	}
}
}