#ifndef GEOS_INDEXSTRTREE_H
#define GEOS_INDEXSTRTREE_H

#include <vector>
#include "platform.h"
#include "geom.h"

using namespace std;

class Boundable {
public:
	virtual void* getBounds()=0;
};

class ItemBoundable: public Boundable {
private:
	void* bounds;
	void* item;
public:
	ItemBoundable(void* newBounds,void* newItem);
	void* getBounds();
	void* getItem();
};

class Interval {
public:
	Interval(Interval *other);
	Interval(double newMin,double newMax);
	double getCentre();
	Interval* expandToInclude(Interval *other);
	bool intersects(Interval *other);
	bool equals(void *o);
private:
	double min;
	double max;
};

/**
 * A node of the STR tree. A leaf node may not have child nodes, but may have
 * child boundables: ItemBoundables.
 */
class AbstractNode: public Boundable {
private:
	vector<Boundable*> *childBoundables;
	void* bounds;
	int level;
public:
	AbstractNode(int newLevel);
	virtual	~AbstractNode();
	vector<Boundable*>* getChildBoundables();
	void* getBounds();
	int getLevel();
	void addChildBoundable(Boundable *childBoundable);
protected:
	virtual void* computeBounds()=0;
};

/**
 * Base class for STRtree and SIRtree. STR-packed R-trees are described in:
 * P. Rigaux, Michel Scholl and Agnes Voisard. Spatial Databases With
 * Application To GIS. Morgan Kaufmann, San Francisco, 2002.
 * @see STRtree
 */
class AbstractSTRtree {
protected:
	static class IntersectsOp {
		public:
			virtual bool intersects(void* aBounds,void* bBounds)=0;
	};
	AbstractNode *root;
	virtual AbstractNode* createNode(int level)=0;
	vector<Boundable*>* createParentBoundables(vector<Boundable*> *childBoundables,int newLevel);
	AbstractNode* lastNode(vector<Boundable*> *nodes);
	int compareDoubles(double a, double b);
	AbstractNode* getRoot();
	void insert(void* bounds,void* item);
	vector<void*>* query(void* searchBounds);
	virtual IntersectsOp* getIntersectsOp()=0;
	vector<Boundable*>* boundablesAtLevel(int level);
private:
	bool built;
	vector<Boundable*> *itemBoundables;
	int nodeCapacity;
	AbstractNode* createHigherLevels(vector<Boundable*> *boundablesOfALevel,int level);
public:
	AbstractSTRtree(int newNodeCapacity);
	virtual ~AbstractSTRtree();
	void build();
	void checkConsistency();
	int getNodeCapacity();
	void query(void* searchBounds,AbstractNode* node,vector<void*>* matches);
	void boundablesAtLevel(int level,AbstractNode* top,vector<Boundable*> *boundables);
//	virtual bool getComparator(Boundable *a, Boundable *b);
};

class SIRtree {
public:
	SIRtree();
	void insert(double a,double b,LineSegment *ls);
};
	
class STRtree {
public:
	STRtree();
	void insert(double a,double b,LineSegment *ls);
};


#endif