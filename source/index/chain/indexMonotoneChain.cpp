/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/indexChain.h>
#include <stdio.h>
#include <geos/util.h>

namespace geos {

indexMonotoneChain::indexMonotoneChain(CoordinateSequence *newPts,int nstart,int nend, void* nContext):
	pts(newPts),
	start(nstart),
	end(nend),
	env(NULL),
	context(nContext)
{
}

void indexMonotoneChain::setId(int nId) {
	id=nId;
}

int
indexMonotoneChain::getId() {
	return id;
}

void* indexMonotoneChain::getContext() {
	return context;
}

indexMonotoneChain::~indexMonotoneChain() {
	delete env;
}

Envelope* indexMonotoneChain::getEnvelope() {
	if (env==NULL) {
		const Coordinate& p0=pts->getAt(start);
		const Coordinate& p1=pts->getAt(end);
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
CoordinateSequence* indexMonotoneChain::getCoordinates() {
	return pts->clone();
}

/**
* Determine all the line segments in the chain whose envelopes overlap
* the searchEnvelope,and process them
*/
void indexMonotoneChain::select(Envelope *searchEnv,MonotoneChainSelectAction *mcs) {
	computeSelect(searchEnv,start,end,mcs);
}

void indexMonotoneChain::computeSelect(Envelope *searchEnv,int start0,int end0,MonotoneChainSelectAction *mcs ) {
	const Coordinate& p0=pts->getAt(start0);
	const Coordinate& p1=pts->getAt(end0);
	mcs->tempEnv1->init(p0,p1);
	//Debug.println("trying:"+p0+p1+" [ "+start0+","+end0+" ]");
	// terminating condition for the recursion
	if(end0-start0==1) {
		//Debug.println("computeSelect:"+p0+p1);
		mcs->select(this,start0);
		return;
	}
	// nothing to do if the envelopes don't overlap
	if (!searchEnv->intersects(mcs->tempEnv1))
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

void
indexMonotoneChain::computeOverlaps(indexMonotoneChain *mc, MonotoneChainOverlapAction *mco)
{
	computeOverlaps(start,end,mc,mc->start,mc->end,mco);
}

void
indexMonotoneChain::computeOverlaps(int start0,int end0,indexMonotoneChain *mc,int start1,int end1,MonotoneChainOverlapAction *mco)
{
	const Coordinate& p00=pts->getAt(start0);
	const Coordinate& p01=pts->getAt(end0);
	const Coordinate& p10=mc->pts->getAt(start1);
	const Coordinate& p11=mc->pts->getAt(end1);
	//Debug.println("computeIntersectsForChain:"+p00+p01+p10+p11);
	// terminating condition for the recursion
	if (end0-start0==1 && end1-start1==1) {
		mco->overlap(this,start0,mc,start1);
		return;
	}
	// nothing to do if the envelopes of these chains don't overlap
	mco->tempEnv1->init(p00,p01);
	mco->tempEnv2->init(p10,p11);
	if (!mco->tempEnv1->intersects(mco->tempEnv2)) return;
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

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.13  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.12  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.11  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.10  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.9  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

