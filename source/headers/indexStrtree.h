#ifndef GEOS_INDEXSTRTREE_H
#define GEOS_INDEXSTRTREE_H

#include <memory>
#include <vector>
#include "platform.h"
#include "spatialIndex.h"
#include "geom.h"

using namespace std;

namespace geos {

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
	double imin;
	double imax;
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
	class IntersectsOp {
		public:
			virtual bool intersects(void* aBounds,void* bBounds)=0;
	};
	AbstractNode *root;
	virtual AbstractNode* createNode(int level)=0;
	virtual vector<Boundable*>* createParentBoundables(vector<Boundable*> *childBoundables,int newLevel);
	virtual AbstractNode* lastNode(vector<Boundable*> *nodes);
	virtual AbstractNode* getRoot();
	virtual void insert(void* bounds,void* item);
	virtual vector<void*>* query(void* searchBounds);
//	virtual IntersectsOp* getIntersectsOp()=0;
	virtual vector<Boundable*>* boundablesAtLevel(int level);
	int nodeCapacity;
private:
	bool built;
	vector<Boundable*> *itemBoundables;
	virtual AbstractNode* createHigherLevels(vector<Boundable*> *boundablesOfALevel,int level);
public:
	IntersectsOp* intersectsOp;
	AbstractSTRtree(int newNodeCapacity);
	static int compareDoubles(double a, double b);
	virtual ~AbstractSTRtree();
	virtual void build();
	virtual void checkConsistency();
	virtual int getNodeCapacity();
	virtual void query(void* searchBounds,AbstractNode* node,vector<void*>* matches);
	virtual void boundablesAtLevel(int level,AbstractNode* top,vector<Boundable*> *boundables);
//	virtual bool getComparator(Boundable *a, Boundable *b);
};

class SIRAbstractNode: public AbstractNode{
public:
	SIRAbstractNode(int level);
protected:
	void* computeBounds();
};
/**
 * One-dimensional version of an STR-packed R-tree. SIR stands for
 * "Sort-Interval-Recursive". STR-packed R-trees are described in:
 * P. Rigaux, Michel Scholl and Agnes Voisard. Spatial Databases With
 * Application To GIS. Morgan Kaufmann, San Francisco, 2002.
 * @see STRtree
 */
class SIRtree: public AbstractSTRtree {
public:
	SIRtree();
	SIRtree(int nodeCapacity);
	virtual ~SIRtree();
	void insert(double x1,double x2,void* item);
	vector<void*>* query(double x);
	vector<void*>* query(double x1, double x2);
protected:
	class SIRIntersectsOp:public AbstractSTRtree::IntersectsOp {
		public:
			bool intersects(void* aBounds,void* bBounds);
	};
	vector<Boundable*>* createParentBoundables(vector<Boundable*> *childBoundables,int newLevel);
	AbstractNode* createNode(int level);
//	IntersectsOp* getIntersectsOp(){return NULL;);
};
	
//Not used yet, thus not ported.

/**
 *  An R-tree created using the Sort-Tile-Recursive (STR) algorithm, described
 *  in: P. Rigaux, Michel Scholl and Agnes Voisard. Spatial Databases With
 *  Application To GIS. Morgan Kaufmann, San Francisco, 2002. <P>
 *
 *  The STR packed R-tree is simple to implement and maximizes space
 *  utilization; that is, as many leaves as possible are filled to capacity.
 *  Overlap between nodes is far less than in a basic R-tree. However, once the
 *  tree has been built (explicitly or on the first call to #query), items may
 *  not be added or removed. <P>
 *
 *  This implementation is based on Rectangles rather than Nodes, because the
 *  STR algorithm operates on both nodes and items, both of which are treated
 *  here as Rectangles (using the Composite design pattern). [Jon Aquino]
 */
class STRtree: public AbstractSTRtree,public SpatialIndex {
//private:
//	Comparator* xComparator;
//	Comparator* yComparator;
//	double centreX(Envelope *e);
//	double avg(double a, double b);
//	double centreY(Envelope *e);
//	IntersectsOp* intersectsOp;
//	vector<Boundable*>* createParentBoundables(vector<Boundable*> *childBoundables, int newLevel);
//	vector<Boundable*>* createParentBoundablesFromVerticalSlices(vector<vector<Boundable*>*>* verticalSlices, int newLevel);
//protected:
//	Comparator* getComparator();
//	vector<Boundable*>* createParentBoundablesFromVerticalSlice(vector<Boundable*> *childBoundables, int newLevel);
//	vector<vector<Boundable*>*>* verticalSlices(vector<Boundable*> *childBoundables, int sliceCount);
//	AbstractNode* createNode(int level)
//	IntersectsOp* getIntersectsOp();
public:
	STRtree();
//	STRtree(int nodeCapacity);
//	void insert(Envelope *itemEnv,void* item);
//	vector<void*>* query(Envelope *searchEnv);
};
}
#endif

