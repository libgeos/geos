#include "../../headers/indexStrtree.h"
#include "../../headers/util.h"
#include "stdio.h"
#include <algorithm>
#include <typeinfo>

namespace geos {

AbstractSTRtree::AbstractSTRtree(int newNodeCapacity) {
	built=false;
	itemBoundables=new vector<Boundable*>();
	Assert::isTrue(newNodeCapacity>1, "Node capacity must be greater than 1");
	nodeCapacity=newNodeCapacity;
}

AbstractSTRtree::~AbstractSTRtree() {
	delete itemBoundables;
}

void AbstractSTRtree::build() {
	Assert::isTrue(!built);
	root=(itemBoundables->empty()?createNode(0):createHigherLevels(itemBoundables,-1));
	built=true;
}

void AbstractSTRtree::checkConsistency() {
	if (!built) {
		build();
	}
	vector<Boundable*>* itemBoundablesInTree=boundablesAtLevel(-1);
	Assert::isTrue(itemBoundables->size()==itemBoundablesInTree->size());
}

bool compareAbsBoundables(Boundable *a, Boundable *b){
	return false;
}

/**
* Sorts the childBoundables then divides them into groups of size M, where
* M is the node capacity.
*/
vector<Boundable*>* AbstractSTRtree::createParentBoundables(vector<Boundable*> *childBoundables,int newLevel) {
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
*  Creates the levels higher than the given level
*
*@param  boundablesOfALevel  the level to build on
*@param  level               the level of the Boundables, or -1 if the
*      boundables are item boundables (that is, below level 0)
*@return                     the root, which may be a ParentNode or a
*      LeafNode
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
		if (typeid(*childBoundable)==typeid(AbstractNode)) {
			query(searchBounds,(AbstractNode*)childBoundable,matches);
		} else if (typeid(*childBoundable)==typeid(ItemBoundable)) {
			matches->push_back(((ItemBoundable*)childBoundable)->getItem());
		} else {
			Assert::shouldNeverReachHere();
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

