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
 **********************************************************************
 * $Log$
 * Revision 1.6  2004/03/25 02:23:55  ybychkov
 * All "index/*" packages upgraded to JTS 1.4
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/indexStrtree.h"
#include "../../headers/util.h"

namespace geos {

bool compareSIRBoundables(Boundable *a, Boundable *b){
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
	vector<Boundable*> *sortedChildBoundables=new vector<Boundable*>(childBoundables->begin(),childBoundables->end());
	sort(sortedChildBoundables->begin(),sortedChildBoundables->end(),compareSIRBoundables);
	for(int i=0;i<(int)sortedChildBoundables->size();i++) {
		Boundable *childBoundable=(AbstractNode*)(*sortedChildBoundables)[i];
		if (lastNode(parentBoundables)->getChildBoundables()->size()==nodeCapacity) {
			parentBoundables->push_back(createNode(newLevel));
		}
		lastNode(parentBoundables)->addChildBoundable(childBoundable);
	}
	return parentBoundables;
}

bool SIRtree::SIRIntersectsOp::intersects(void* aBounds,void* bBounds) {
	return ((Interval*)aBounds)->intersects((Interval*)bBounds);
}

/**
* Constructs an SIRtree with the default node capacity.
*/
SIRtree::SIRtree(): AbstractSTRtree(10){
	intersectsOp=new SIRIntersectsOp();
}

/**
* Constructs an SIRtree with the given maximum number of child nodes that
* a node may have
*/
SIRtree::SIRtree(int nodeCapacity): AbstractSTRtree(nodeCapacity){
	intersectsOp=new SIRIntersectsOp();
}

SIRtree::~SIRtree() {
	delete intersectsOp;
}

SIRAbstractNode::SIRAbstractNode(int level):AbstractNode(level) {}

void* SIRAbstractNode::computeBounds() {
	Interval* bounds=NULL;
	vector<Boundable*> *b=getChildBoundables();
	for(int i=0;i<(int)b->size();i++) {
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
	return new SIRAbstractNode(level);
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
}

