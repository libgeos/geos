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

#include <geos/indexStrtree.h>
#include <geos/util.h>
#include <geos/profiler.h>

namespace geos {


static bool xComparator(Boundable *a, Boundable *b){
	return AbstractSTRtree::compareDoubles(STRtree::centreX((Envelope*)a->getBounds()), STRtree::centreX((Envelope*)b->getBounds()));
}

static bool yComparator(Boundable *a, Boundable *b){
	return AbstractSTRtree::compareDoubles(STRtree::centreY((Envelope*)a->getBounds()), STRtree::centreY((Envelope*)b->getBounds()));
}

/**
 * Constructs an STRtree with the default node capacity.
 */
STRtree::STRtree():
	AbstractSTRtree(10)
{ 
	//intersectsOp(new STRIntersectsOp())
}

/**
 * Constructs an STRtree with the given maximum number of child nodes that
 * a node may have
 */
STRtree::STRtree(int nodeCapacity):
	AbstractSTRtree(nodeCapacity)
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

double STRtree::avg(double a, double b) { 
	return (a + b) / 2.0;
}

double STRtree::centreY(Envelope *e) {
	return STRtree::avg(e->getMinY(), e->getMaxY());
}


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
	for (unsigned int i=0; i<verticalSlicesV->size(); i++)
	{
		vector<Boundable *>*inner = (*verticalSlicesV)[i];
		//for (unsigned int j=0; j<inner->size(); j++)
		//{
			// some of these might be provided,
			// some of these might be created
			//delete (*inner)[j];
		//}
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
	for (unsigned int i = 0; i <verticalSlices->size(); i++) {
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
	for (int j=0; j<sliceCount; j++) {
		(*slices)[j]=new vector<Boundable*>();
		int boundablesAddedToSlice = 0;
		while (i<childBoundables->size() && boundablesAddedToSlice < sliceCapacity)
		{
			Boundable *childBoundable=(*childBoundables)[i];
			i++;
			(*slices)[j]->push_back(childBoundable);
			boundablesAddedToSlice++;
		}
	}
	return slices;
}

STRAbstractNode::STRAbstractNode(int level):AbstractNode(level)
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
	AbstractNode *an = new STRAbstractNode(level);
	nodes->push_back(an);
	return an;
}

void
STRtree::insert(const Envelope *itemEnv, void* item)
{
	if (itemEnv->isNull()) { return; }
	AbstractSTRtree::insert(itemEnv, item);
}

vector<void*>*
STRtree::query(const Envelope *searchEnv)
{
	vector<void *> *ret = AbstractSTRtree::query(searchEnv);
	return ret;
}

vector<Boundable*> *
STRtree::sortBoundables(const vector<Boundable*> *input)
{
	vector<Boundable*> *output=new vector<Boundable*>(*input);
	sort(output->begin(),output->end(),yComparator);
	return output;
}

} // namespace geos

/**********************************************************************
 * $Log$
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
 * Revision 1.12  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.11  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/05/06 15:00:59  strk
 * Boundable destructor made virtual.
 * Added vector <AbstractNode *> *nodes member in AbstractSTRTree,
 * used to keep track of created node to cleanly delete them at
 * destruction time.
 *
 * Revision 1.9  2004/05/06 13:58:30  strk
 * leak removed from createParentBoundablesFromVerticalSlices
 *
 * Revision 1.8  2004/05/05 17:42:06  strk
 * AbstractNode destructor made virtual. AbstractNode::bounds made protected.
 * SIRAbstractNode and STRAbstractNode destructors added to get rid of
 * AbstractNode::bounds in the right way (is a void * casted to appropriate
 * Class in the subClasses).
 *
 * Revision 1.7  2004/05/03 17:15:38  strk
 * leaks on exception fixed.
 *
 * Revision 1.6  2004/05/03 16:29:21  strk
 * Added sortBoundables(const vector<Boundable *>) pure virtual in AbstractSTRtree,
 * implemented in SIRtree and STRtree. Comparator funx made static in STRtree.cpp
 * and SIRtree.cpp.
 *
 * Revision 1.5  2004/04/26 12:37:19  strk
 * Some leaks fixed.
 *
 * Revision 1.4  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 * Revision 1.3  2004/03/25 02:23:55  ybychkov
 * All "index/*" packages upgraded to JTS 1.4
 *
 * Revision 1.2  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

