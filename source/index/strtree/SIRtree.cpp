#include "../../headers/indexStrtree.h"
#include "../../headers/util.h"


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

SIRtree::SIRtree(): AbstractSTRtree(10){
	intersectsOp=new SIRIntersectsOp();
}

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

void SIRtree::insert(double x1, double x2,void* item) {
	AbstractSTRtree::insert(new Interval(__min(x1,x2),__max(x1, x2)),item);
}

vector<void*>* SIRtree::query(double x) {
	return query(x, x);
}

/**
* min and max may be the same value
*/
vector<void*>* SIRtree::query(double x1, double x2) {
	return AbstractSTRtree::query(new Interval(__min(x1, x2),__max(x1, x2)));
}