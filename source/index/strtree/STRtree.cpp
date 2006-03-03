/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/indexStrtree.h>
#include <geos/util.h>
#include <geos/profiler.h>
#include <vector>

using namespace std;

namespace geos {
namespace index { // geos.index
namespace strtree { // geos.index.strtree


//static bool xComparator(Boundable *a, Boundable *b){
	//return AbstractSTRtree::compareDoubles(STRtree::centreX((Envelope*)a->getBounds()), STRtree::centreX((Envelope*)b->getBounds()));
//}

static bool yComparator(Boundable *a, Boundable *b){
	//return AbstractSTRtree::compareDoubles(STRtree::centreY((Envelope*)a->getBounds()), STRtree::centreY((Envelope*)b->getBounds()));
	return STRtree::centreY((Envelope*)a->getBounds()) < STRtree::centreY((Envelope*)b->getBounds());
}

/**
 * Constructs an STRtree with the given maximum number of child nodes that
 * a node may have
 */
STRtree::STRtree(int nodeCapacity): AbstractSTRtree(nodeCapacity)
{ 
	//intersectsOp(new STRIntersectsOp())
}

STRtree::~STRtree()
{ 
	//delete intersectsOp;
}

double STRtree::centreX(Envelope *e) {
	return STRtree::avg(e->getMinX(),e->getMaxX());
}

//double STRtree::avg(double a, double b) { 
//	return (a + b) / 2.0;
//}

//double STRtree::centreY(Envelope *e) {
	//return STRtree::avg(e->getMinY(), e->getMaxY());
//}


bool
STRtree::STRIntersectsOp::intersects(const void* aBounds, const void* bBounds)
{
	return ((Envelope*)aBounds)->intersects((Envelope*)bBounds);
}

/**
 * Creates the parent level for the given child level. First, orders the items
 * by the x-values of the midpoints, and groups them into vertical slices.
 * For each slice, orders the items by the y-values of the midpoints, and
 * group them into runs of size M (the node capacity). For each run, creates
 * a new (parent) node.
 */
vector<Boundable*>*
STRtree::createParentBoundables(vector<Boundable*> *childBoundables, int newLevel)
{
	Assert::isTrue(!childBoundables->empty());
	int minLeafCount=(int) ceil((double)childBoundables->size()/(double)getNodeCapacity());

	vector<Boundable*> *sortedChildBoundables=sortBoundables(childBoundables);

	vector<vector<Boundable*>*>* verticalSlicesV = verticalSlices(sortedChildBoundables,(int)ceil(sqrt((double)minLeafCount)));
	delete sortedChildBoundables;

	vector<Boundable*> *ret;
	ret = createParentBoundablesFromVerticalSlices(verticalSlicesV, newLevel);
	unsigned int vssize=verticalSlicesV->size();
	for (unsigned int i=0; i<vssize; i++)
	{
		vector<Boundable *>*inner = (*verticalSlicesV)[i];
		delete inner;
	}

	delete verticalSlicesV;

	return ret;
}

vector<Boundable*>*
STRtree::createParentBoundablesFromVerticalSlices(vector<vector<Boundable*>*> *verticalSlices, int newLevel)
{
	Assert::isTrue(verticalSlices->size()>0);
	vector<Boundable*> *parentBoundables=new vector<Boundable*>();

	unsigned int vssize=verticalSlices->size();
	for (unsigned int i = 0; i<vssize; i++) {
		vector<Boundable*> *toAdd=createParentBoundablesFromVerticalSlice((*verticalSlices)[i], newLevel);
		parentBoundables->insert(parentBoundables->end(),toAdd->begin(),toAdd->end());
		delete toAdd;
	}
	return parentBoundables;
}

vector<Boundable*>*
STRtree::createParentBoundablesFromVerticalSlice(vector<Boundable*> *childBoundables, int newLevel)
{
	return AbstractSTRtree::createParentBoundables(childBoundables, newLevel);
}

/**
 * @param childBoundables Must be sorted by the x-value of
 *        the envelope midpoints
 * @return
 */
vector<vector<Boundable*>*>*
STRtree::verticalSlices(vector<Boundable*>* childBoundables, int sliceCount)
{
	int sliceCapacity = (int) ceil((double)childBoundables->size() / (double) sliceCount);
	vector<vector<Boundable*>*>* slices = new vector<vector<Boundable*>*>(sliceCount);

	unsigned int i=0;
	unsigned int nchilds=childBoundables->size();


	for (int j=0; j<sliceCount; j++) {
		(*slices)[j]=new vector<Boundable*>();
		(*slices)[j]->reserve(sliceCapacity);
		int boundablesAddedToSlice = 0;
		while (i<nchilds && boundablesAddedToSlice<sliceCapacity)
		{
			Boundable *childBoundable=(*childBoundables)[i];
			i++;
			(*slices)[j]->push_back(childBoundable);
			boundablesAddedToSlice++;
		}
	}
	return slices;
}

STRAbstractNode::STRAbstractNode(int level, int cpcty):
	AbstractNode(level, cpcty)
{
}

STRAbstractNode::~STRAbstractNode()
{
	delete (Envelope *)bounds;
}

void *
STRAbstractNode::computeBounds()
{
	Envelope* bounds=NULL;
	vector<Boundable*> *b=getChildBoundables();
	unsigned int bsize=b->size();

	if ( bsize ) bounds=new Envelope(*(Envelope*)(*b)[0]->getBounds());
	for(unsigned int i=1; i<bsize; i++) {
		Boundable* childBoundable=(*b)[i];
		bounds->expandToInclude((Envelope*)childBoundable->getBounds());
	}
	return bounds;
}

AbstractNode*
STRtree::createNode(int level)
{
	AbstractNode *an = new STRAbstractNode(level, nodeCapacity);
	nodes->push_back(an);
	return an;
}

void
STRtree::insert(const Envelope *itemEnv, void* item)
{
	if (itemEnv->isNull()) { return; }
	AbstractSTRtree::insert(itemEnv, item);
}

vector<Boundable*> *
STRtree::sortBoundables(const vector<Boundable*> *input)
{
	vector<Boundable*> *output=new vector<Boundable*>(*input);
	sort(output->begin(),output->end(),yComparator);
	return output;
}

} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.26  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.25  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.24  2006/02/20 21:04:37  strk
 * - namespace geos::index
 * - SpatialIndex interface synced
 *
 * Revision 1.23  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.22  2005/02/22 18:21:46  strk
 * Changed SegmentNode to contain a *real* Coordinate (not a pointer) to reduce
 * construction costs.
 *
 * Revision 1.21  2005/02/22 15:16:30  strk
 * STRtree::avg() and STRtree::centreY() inlined.
 *
 * Revision 1.20  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.19  2005/01/31 15:41:03  strk
 * Small optimizations.
 *
 * Revision 1.18  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.17  2004/11/08 18:33:47  strk
 * Just another small improvement.
 *
 * Revision 1.16  2004/11/08 15:58:13  strk
 * More performance tuning.
 *
 * Revision 1.15  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.14  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.13  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 **********************************************************************/

