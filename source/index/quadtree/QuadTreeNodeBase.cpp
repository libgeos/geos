#include "../../headers/indexQuadtree.h"

namespace geos {

/**
* Returns the index of the subquad that wholly contains the given envelope.
* If none does, returns -1.
*/
int QuadTreeNodeBase::getSubnodeIndex(Envelope *env,Coordinate *centre){
	int subnodeIndex=-1;
	if (env->getMinX()>=centre->x) {
		if (env->getMinY()>=centre->y) subnodeIndex=3;
		if (env->getMaxY()<=centre->y) subnodeIndex=1;
	}
	if (env->getMaxX()<=centre->x) {
		if (env->getMinY()>=centre->y) subnodeIndex=2;
		if (env->getMaxY()<=centre->y) subnodeIndex=0;
	}
	return subnodeIndex;
}

QuadTreeNodeBase::QuadTreeNodeBase() {
	items=new vector<void*>();
	subnode[0]=NULL;
	subnode[1]=NULL;
	subnode[2]=NULL;
	subnode[3]=NULL;
}

QuadTreeNodeBase::~QuadTreeNodeBase() {
	delete subnode[0];
	delete subnode[1];
	delete subnode[2];
	delete subnode[3];
	subnode[0]=NULL;
	subnode[1]=NULL;
	subnode[2]=NULL;
	subnode[3]=NULL;
	delete items;
}

vector<void*>* QuadTreeNodeBase::getItems() {
	return items;
}

void QuadTreeNodeBase::add(void* item) {
	items->push_back(item);
	//DEBUG itemCount++;
	//DEBUG System.out.print(itemCount);
}

vector<void*>* QuadTreeNodeBase::addAllItems(vector<void*> *resultItems) {
	//<<TODO:ASSERT?>> Can we assert that this node cannot have both items
	//and subnodes? [Jon Aquino]
	resultItems->insert(resultItems->end(),items->begin(),items->end());
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subnode[i]->addAllItems(resultItems);
		}
	}
	return resultItems;
}

void QuadTreeNodeBase::addAllItemsFromOverlapping(Envelope *searchEnv,vector<void*> *resultItems){
	if (!isSearchMatch(searchEnv))
		return;

	//<<TODO:ASSERT?>> Can we assert that this node cannot have both items
	//and subnodes? [Jon Aquino]
	resultItems->insert(resultItems->end(),items->begin(),items->end());
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subnode[i]->addAllItemsFromOverlapping(searchEnv,resultItems);
		}
	}
}

//<<TODO:RENAME?>> In Samet's terminology, I think what we're returning here is
//actually level+1 rather than depth. (See p. 4 of his book) [Jon Aquino]
int QuadTreeNodeBase::depth() {
	int maxSubDepth=0;
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			int sqd=subnode[i]->depth();
			if (sqd>maxSubDepth)
				maxSubDepth=sqd;
		}
	}
	return maxSubDepth+1;
}

//<<TODO:RENAME?>> "size" is a bit generic. How about "itemCount"? [Jon Aquino]
int QuadTreeNodeBase::size() {
	int subSize=0;
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subSize+=subnode[i]->size();
		}
	}
	return subSize+(int)items->size();
}

//<<TODO:RENAME?>> The Java Language Specification recommends that "Methods to
//get and set an attribute that might be thought of as a variable V should be
//named getV and setV" (6.8.3). Perhaps this and other methods should be
//renamed to "get..."? [Jon Aquino]
int QuadTreeNodeBase::nodeCount() {
	int subSize=0;
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subSize+=subnode[i]->size();
		}
	}
	return subSize+1;
}
}

