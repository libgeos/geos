#include "../../headers/indexIntervaltree.h"
#include "stdio.h"

IntervalTree::IntervalTree(double min,double max) {
	root=new IntervalNode(NULL,min,max);
}

IntervalTree::~IntervalTree() {
	delete root;
}

IntervalNode* IntervalTree::getRoot() {
	return root;
}

void IntervalTree::insert(double x1,double x2,void* item) {
	double qmin=min(x1,x2);
	double qmax=max(x1,x2);
	/**
	* Do NOT create a new interval for zero-length intervals - this would lead
	* to infinite recursion. Instead, use a heuristic of simply returning
	* the smallest existing interval containing the query
	*/
	bool isValid=qmax>qmin;
	IntervalNode* interval;
	if (isValid)
		interval=root->getIntervalNode(qmin,qmax);
	else
		interval=root->find(qmin,qmax);
	interval->add(item);
}

vector<void*>* IntervalTree::query(double x) {
	return query(x,x);
}

/**
* min and max may be the same value
*/
vector<void*>* IntervalTree::query(double x1, double x2) {
	double qmin=min(x1,x2);
	double qmax=max(x1,x2);
	/**
	* the items that are matched are all items in intervals
	* which overlap the query interval
	*/
	vector<void*>* foundItems=new vector<void*>();
	root->addAllItemsFromOverlapping(x1,x2,foundItems);
	return foundItems;
}
