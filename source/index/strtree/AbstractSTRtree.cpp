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
 * Revision 1.9  2004/04/28 14:58:47  strk
 * Made AbstractSTRtree::query use dynamic_cast<> to simulate java's
 * instanceof. Previous typeid(*) use missed to catch an STRAbstractNode
 * as a class derived from AbstractNode. Still have to check if this
 * is the correct semantic with Martin, but at least lots of SIGABORT
 * are no more raised.
 *
 * Revision 1.8  2004/04/26 12:37:19  strk
 * Some leaks fixed.
 *
 * Revision 1.7  2004/04/14 12:28:43  strk
 * shouldNeverReachHere exceptions made more verbose
 *
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
#include "stdio.h"
#include <algorithm>
#include <typeinfo>

namespace geos {

/**
* Constructs an AbstractSTRtree with the specified maximum number of child
* nodes that a node may have
*/
AbstractSTRtree::AbstractSTRtree(int newNodeCapacity) {
	built=false;
	itemBoundables=new vector<Boundable*>();
	Assert::isTrue(newNodeCapacity>1, "Node capacity must be greater than 1");
	nodeCapacity=newNodeCapacity;
}

AbstractSTRtree::~AbstractSTRtree() {
	for (int i=0; i<itemBoundables->size(); i++)
	{
		delete (*itemBoundables)[i];
	}
	delete itemBoundables;
}

/**
* Creates parent nodes, grandparent nodes, and so forth up to the root
* node, for the data that has been inserted into the tree. Can only be
* called once, and thus can be called only after all of the data has been
* inserted into the tree.
*/
void AbstractSTRtree::build() {
	Assert::isTrue(!built);
	root=(itemBoundables->empty()?createNode(0):createHigherLevels(itemBoundables,-1));
	built=true;
}

//void AbstractSTRtree::checkConsistency() {
//	if (!built) {
//		build();
//	}
//	vector<Boundable*>* itemBoundablesInTree=boundablesAtLevel(-1);
//	Assert::isTrue(itemBoundables->size()==itemBoundablesInTree->size());
//}

bool compareAbsBoundables(Boundable *a, Boundable *b){
	return false;
}

/**
* Sorts the childBoundables then divides them into groups of size M, where
* M is the node capacity.
*/
vector<Boundable*>*
AbstractSTRtree::createParentBoundables(vector<Boundable*> *childBoundables,int newLevel)
{
	Assert::isTrue(!childBoundables->empty());
	vector<Boundable*> *parentBoundables=new vector<Boundable*>();
	parentBoundables->push_back(createNode(newLevel));
	vector<Boundable*> *sortedChildBoundables=new vector<Boundable*>(childBoundables->begin(),childBoundables->end());
	sort(sortedChildBoundables->begin(),sortedChildBoundables->end(),compareAbsBoundables);
	for(int i=0;i<(int)sortedChildBoundables->size();i++) {
		Boundable *childBoundable=(AbstractNode*)(*sortedChildBoundables)[i];
		if (lastNode(parentBoundables)->getChildBoundables()->size()==nodeCapacity) {
			parentBoundables->push_back(createNode(newLevel));
		}
		lastNode(parentBoundables)->addChildBoundable(childBoundable);
	}
	return parentBoundables;
}

AbstractNode* AbstractSTRtree::lastNode(vector<Boundable*> *nodes) {
	return (AbstractNode*)(*nodes)[nodes->size()-1];
}

int AbstractSTRtree::compareDoubles(double a, double b) {
	return a>b?1:(a<b?-1:0);
}

/**
* Creates the levels higher than the given level
* 
* @param boundablesOfALevel
*            the level to build on
* @param level
*            the level of the Boundables, or -1 if the boundables are item
*            boundables (that is, below level 0)
* @return the root, which may be a ParentNode or a LeafNode
*/
AbstractNode* AbstractSTRtree::createHigherLevels(vector<Boundable*> *boundablesOfALevel,int level) {
	Assert::isTrue(!boundablesOfALevel->empty());
	vector<Boundable*> *parentBoundables=createParentBoundables(boundablesOfALevel,level+1);
	if (parentBoundables->size()==1) {
		return (AbstractNode*)(*parentBoundables)[0];
	}
	return createHigherLevels(parentBoundables,level+1);
}

AbstractNode* AbstractSTRtree::getRoot() {
	return root;
}

/**
* Returns the maximum number of child nodes that a node may have
*/
int AbstractSTRtree::getNodeCapacity() {
	return nodeCapacity;
}

void AbstractSTRtree::insert(void* bounds,void* item) {
	Assert::isTrue(!built,"Cannot insert items into an STR packed R-tree after it has been built.");
	itemBoundables->push_back(new ItemBoundable(bounds,item));
}

/**
*  Also builds the tree, if necessary.
*/
vector<void*>* AbstractSTRtree::query(void* searchBounds) {
	if (!built) {
		build();
	}
	vector<void*> *matches=new vector<void*>();
	if (itemBoundables->empty()) {
		Assert::isTrue(root->getBounds()==NULL);
		return matches;
	}
	if (intersectsOp->intersects(root->getBounds(),searchBounds)) {
		query(searchBounds,root,matches);
	}
	return matches;
}

void AbstractSTRtree::query(void* searchBounds,AbstractNode* node,vector<void*> *matches) {
	vector<Boundable*> *vb=node->getChildBoundables();
	for(int i=0;i<(int)vb->size();i++) {
		Boundable *childBoundable=(*vb)[i];
		if (!intersectsOp->intersects(childBoundable->getBounds(),searchBounds)) {
			continue;
		}

		if(AbstractNode *an=dynamic_cast<AbstractNode*>(childBoundable))
		{
			query(searchBounds,an,matches);
		} else if (ItemBoundable *ib=dynamic_cast<ItemBoundable *>(childBoundable))
		{
			matches->push_back(ib->getItem());
		} else {
			Assert::shouldNeverReachHere("AbstractSTRtree::query encountered an unsupported childBoundable type");
		}
	}
}

vector<Boundable*>* AbstractSTRtree::boundablesAtLevel(int level) {
	vector<Boundable*> *boundables=new vector<Boundable*>();
	boundablesAtLevel(level,root,boundables);
	return boundables;
}

/**
* @param level -1 to get items
*/
void AbstractSTRtree::boundablesAtLevel(int level,AbstractNode* top,vector<Boundable*> *boundables) {
	Assert::isTrue(level>-2);
	if (top->getLevel()==level) {
		boundables->push_back(top);
		return;
	}
	vector<Boundable*> *vb=top->getChildBoundables();
	for(int i=0;i<(int)vb->size();i++) {
		Boundable *boundable=(*vb)[i];
		if (typeid(*boundable)==typeid(AbstractNode)) {
			boundablesAtLevel(level,(AbstractNode*) boundable,boundables);
		} else {
			Assert::isTrue(typeid(*boundable)==typeid(ItemBoundable));
			if (level==-1) {
				boundables->push_back(boundable);
			}
		}
	}
	return;
}
}

