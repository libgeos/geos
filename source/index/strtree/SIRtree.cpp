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

namespace geos {

static bool compareSIRBoundables(Boundable *a, Boundable *b){
	return AbstractSTRtree::compareDoubles(((Interval*)a->getBounds())->getCentre(),((Interval*)b->getBounds())->getCentre());
}

/**
* Sorts the childBoundables then divides them into groups of size M, where
* M is the node capacity.
*/
vector<Boundable*>* SIRtree::createParentBoundables(vector<Boundable*> *childBoundables,int newLevel) {
	Assert::isTrue(!childBoundables->empty());
	vector<Boundable*> *parentBoundables=new vector<Boundable*>();
	parentBoundables->push_back(createNode(newLevel));

	vector<Boundable*> *sortedChildBoundables=sortBoundables(childBoundables);

	for(unsigned int i=0;i<sortedChildBoundables->size();i++) {
		Boundable *childBoundable=(AbstractNode*)(*sortedChildBoundables)[i];
		if (lastNode(parentBoundables)->getChildBoundables()->size()==(unsigned int)nodeCapacity) {
			parentBoundables->push_back(createNode(newLevel));
		}
		lastNode(parentBoundables)->addChildBoundable(childBoundable);
	}
	return parentBoundables;
}

bool
SIRtree::SIRIntersectsOp::intersects(const void* aBounds, const void* bBounds)
{
	return ((Interval*)aBounds)->intersects((Interval*)bBounds);
}

/**
* Constructs an SIRtree with the default node capacity.
*/
SIRtree::SIRtree():
	AbstractSTRtree(10),
	intersectsOp(new SIRIntersectsOp())
{
}

/**
* Constructs an SIRtree with the given maximum number of child nodes that
* a node may have
*/
SIRtree::SIRtree(int nodeCapacity):
	AbstractSTRtree(nodeCapacity),
	intersectsOp(new SIRIntersectsOp())
{
}

SIRtree::~SIRtree() {
	delete intersectsOp;
}

SIRAbstractNode::SIRAbstractNode(int level, int capacity):
	AbstractNode(level, capacity)
{}

SIRAbstractNode::~SIRAbstractNode()
{
	delete (Interval *)bounds;
}

void* SIRAbstractNode::computeBounds() {
	Interval* bounds=NULL;
	vector<Boundable*> *b=getChildBoundables();
	for(unsigned int i=0;i<b->size();i++) {
		Boundable* childBoundable=(*b)[i];
		if (bounds==NULL) {
			bounds=new Interval((Interval*)childBoundable->getBounds());
		} else {
			bounds->expandToInclude((Interval*)childBoundable->getBounds());
		}
	}
	return bounds;
}

AbstractNode* SIRtree::createNode(int level) {
	AbstractNode *an = new SIRAbstractNode(level, nodeCapacity);
	nodes->push_back(an);
	return an;
}

/**
* Inserts an item having the given bounds into the tree.
*/
void SIRtree::insert(double x1, double x2,void* item) {
	AbstractSTRtree::insert(new Interval(min(x1,x2),max(x1, x2)),item);
}

/**
* Returns items whose bounds intersect the given value.
*/
vector<void*>* SIRtree::query(double x) {
	return query(x, x);
}

/**
* Returns items whose bounds intersect the given bounds.
* @param x1 possibly equal to x2
*/
vector<void*>* SIRtree::query(double x1, double x2) {
	return AbstractSTRtree::query(new Interval(min(x1, x2),max(x1, x2)));
}

vector<Boundable*> *
SIRtree::sortBoundables(const vector<Boundable*> *input)
{
	vector<Boundable*> *output=new vector<Boundable*>(*input);
	sort(output->begin(), output->end(), compareSIRBoundables);
	return output;
}

} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.14  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.13  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.12  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.11  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.10  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2004/05/06 15:00:59  strk
 * Boundable destructor made virtual.
 * Added vector <AbstractNode *> *nodes member in AbstractSTRTree,
 * used to keep track of created node to cleanly delete them at
 * destruction time.
 *
 **********************************************************************/

