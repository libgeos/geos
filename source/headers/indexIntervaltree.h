#ifndef GEOS_INDEXINTERVALTREE_H
#define GEOS_INDEXINTERVALTREE_H

#include <vector>
#include "platform.h"
#include "geom.h"

using namespace std;

class IntervalNode {
public:
	IntervalNode(IntervalNode *newParent,double newMin,double newMax);
	~IntervalNode();
	IntervalNode* getParent();
	IntervalNode* getChildren();
	bool overlaps(double qmin,double qmax);
	vector<void*>* getItems();
	void add(void* item);
	/**
	* Collect all items for this node and all nodes below it
	* which overlap the query interval
	*/
	vector<void*>* addAllItemsFromOverlapping(double qmin,double qmax,vector<void*> *newItems);
	/**
	* Returns the interval containing the envelope.
	* Creates the interval if it does not already exist.
	* Note that passing a zero-size interval to this routine results in infinite recursion.
	*/
	IntervalNode* getIntervalNode(double qmin,double qmax);
	/**
	* Returns the smallest existing node containing the envelope.
	*/
	IntervalNode* find(double qmin,double qmax);
private:
	/**
	* Returns the index of the subquad that wholely contains the search envelope.
	* If none does, returns -1
	*/
	int getSubintervalIndex(double qmin,double qmax);
	/**
	* get the subinterval for the index.
	* If it doesn't exist, create it
	*/
	IntervalNode* getSubinterval(int index);
	double min, max;
	double centre;
	IntervalNode *parent;
	vector<void*> *items;
	IntervalNode* subinterval[2];
};

class IntervalTree {
public:
	IntervalTree(double min, double max);
	~IntervalTree();
	IntervalNode* getRoot();
	void insert(double x1,double x2,void* item);
	vector<void*>* query(double x);
	/**
	* min and max may be the same value
	*/
	vector<void*>* query(double x1,double x2);
private:
	IntervalNode* root;
};

#endif

