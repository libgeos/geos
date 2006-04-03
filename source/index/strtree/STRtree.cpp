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
 **********************************************************************
 *
 * Last port: index/strtree/STRtree.java rev. 1.11
 *
 **********************************************************************/

#include <geos/index/strtree/STRtree.h>
#include <geos/geom/Envelope.h>

#include <vector>
#include <cassert>
#include <cmath>

using namespace std;
using namespace geos::geom;

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

/*public*/
STRtree::STRtree(int nodeCapacity): AbstractSTRtree(nodeCapacity)
{ 
}

/*public*/
STRtree::~STRtree()
{ 
}

#if 0
/*public static*/
double
STRtree::centreX(const Envelope *e)
{
	return STRtree::avg(e->getMinX(),e->getMaxX());
}
#endif // 0

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

/*private*/
vector<Boundable*>*
STRtree::createParentBoundables(vector<Boundable*> *childBoundables, int newLevel)
{
	assert(!childBoundables->empty());
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

/*private*/
vector<Boundable*>*
STRtree::createParentBoundablesFromVerticalSlices(vector<vector<Boundable*>*> *verticalSlices, int newLevel)
{
	assert(verticalSlices->size()>0);
	vector<Boundable*> *parentBoundables=new vector<Boundable*>();

	unsigned int vssize=verticalSlices->size();
	for (unsigned int i = 0; i<vssize; i++) {
		vector<Boundable*> *toAdd=createParentBoundablesFromVerticalSlice((*verticalSlices)[i], newLevel);
		parentBoundables->insert(parentBoundables->end(),toAdd->begin(),toAdd->end());
		delete toAdd;
	}
	return parentBoundables;
}

/*protected*/
vector<Boundable*>*
STRtree::createParentBoundablesFromVerticalSlice(vector<Boundable*> *childBoundables, int newLevel)
{
	return AbstractSTRtree::createParentBoundables(childBoundables, newLevel);
}

/*protected*/
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

class STRAbstractNode: public AbstractNode{
public:

	STRAbstractNode(int level, int capacity)
		:
		AbstractNode(level, capacity)
	{}

	~STRAbstractNode()
	{
		delete (Envelope *)bounds;
	}

protected:

	void* computeBounds()
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

};

/*protected*/
AbstractNode*
STRtree::createNode(int level)
{
	AbstractNode *an = new STRAbstractNode(level, nodeCapacity);
	nodes->push_back(an);
	return an;
}

/*public*/
void
STRtree::insert(const Envelope *itemEnv, void* item)
{
	if (itemEnv->isNull()) { return; }
	AbstractSTRtree::insert(itemEnv, item);
}

/*protected*/
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
 * Revision 1.31  2006/04/03 08:43:09  strk
 * Added port info, minor cleanups
 *
 * Revision 1.30  2006/03/21 10:47:34  strk
 * indexStrtree.h split
 *
 * Revision 1.29  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.28  2006/03/15 18:44:52  strk
 * Bug #60 - Missing <cmath> header in some files
 *
 * Revision 1.27  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.26  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 **********************************************************************/

