#include "../../headers/indexIntervaltree.h"
#include "stdio.h"

IntervalNode::IntervalNode(IntervalNode* newParent,double newMin,double newMax){
	parent=newParent;
	min=newMin;
	max=newMax;
	centre=(min+max)/2;
	items=new vector<void*>();
	subinterval[0]=NULL;
	subinterval[1]=NULL;
}

IntervalNode::~IntervalNode(){
	delete items;
}

IntervalNode* IntervalNode::getParent() {
	return parent;
}

IntervalNode* IntervalNode::getChildren() {
	return (IntervalNode*)subinterval;
}

bool IntervalNode::overlaps(double qmin, double qmax) {
	if (qmin>max || qmax<min) return false;
	return true;
}

vector<void*>* IntervalNode::getItems() {
	return items;
}

void IntervalNode::add(void* item) {
	items->push_back(item);
}

/**
* Collect all items for this node and all nodes below it
* which overlap the query interval
*/
vector<void*>* IntervalNode::addAllItemsFromOverlapping(double qmin,double qmax,vector<void*> *newItems) {
	newItems->insert(newItems->end(),items->begin(),items->end());
	for(int i=0;i<2;i++) {
		if(subinterval[i]!=NULL && subinterval[i]->overlaps(qmin,qmax)) {
			subinterval[i]->addAllItemsFromOverlapping(qmin,qmax,newItems);
		}
	}
	return newItems;
}

/**
* Returns the interval containing the envelope.
* Creates the interval if it does not already exist.
* Note that passing a zero-size interval to this routine results in infinite recursion.
*/
IntervalNode* IntervalNode::getIntervalNode(double qmin,double qmax){
	int subintervalIndex=getSubintervalIndex(qmin,qmax);
	if (subintervalIndex!=-1) {
		// create the quad if it does not exist
		IntervalNode* interval=getSubinterval(subintervalIndex);
		// recursively search the found quad
		return interval->getIntervalNode(qmin,qmax);
	} else {
		return this;
	}
}

/**
* Returns the smallest existing node containing the envelope.
*/
IntervalNode* IntervalNode::find(double qmin,double qmax) {
	int subintervalIndex=getSubintervalIndex(qmin,qmax);
	if (subintervalIndex==-1 || subinterval[subintervalIndex]==NULL)
		return this;
	// query lies in subnode, so search it recursively
	IntervalNode* node=subinterval[subintervalIndex];
	return node->find(qmin,qmax);
}

/**
* Returns the index of the subquad that wholely contains the search envelope.
* If none does, returns -1
*/
int IntervalNode::getSubintervalIndex(double qmin,double qmax){
	int subintervalIndex=-1;
	if (min>centre) subintervalIndex=1;
	if (max<centre) subintervalIndex=0;
	return subintervalIndex;
}

/**
* get the subinterval for the index.
* If it doesn't exist, create it
*/
IntervalNode* IntervalNode::getSubinterval(int index) {
	if (subinterval[index]==NULL) {
	// create a new subquad in the appropriate quadrant
		double submin=0.0;
		double submax=0.0;
		switch(index) {
			case 0:
				submin=min;
				submax=centre;
				break;
			case 1:
				submin=centre;
				submax=max;
				break;
		}
		IntervalNode *interval=new IntervalNode(this,submin,submax);
		subinterval[index]=interval;
	}
	return subinterval[index];
}