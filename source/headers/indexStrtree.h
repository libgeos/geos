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
 * Revision 1.12  2004/05/03 16:29:21  strk
 * Added sortBoundables(const vector<Boundable *>) pure virtual in AbstractSTRtree,
 * implemented in SIRtree and STRtree. Comparator funx made static in STRtree.cpp
 * and SIRtree.cpp.
 *
 * Revision 1.11  2004/05/03 13:17:55  strk
 * Fixed comparator function to express StrictWeakOrdering.
 *
 * Revision 1.10  2004/04/05 06:35:14  ybychkov
 * "operation/distance" upgraded to JTS 1.4
 *
 * Revision 1.9  2004/03/25 02:23:55  ybychkov
 * All "index/*" packages upgraded to JTS 1.4
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_INDEXSTRTREE_H
#define GEOS_INDEXSTRTREE_H

#include <memory>
#include <vector>
#include "platform.h"
#include "spatialIndex.h"
#include "geom.h"

using namespace std;

namespace geos {

/**
 * A spatial object in an AbstractSTRtree.
 *
 */
class Boundable {
public:
  /**
   * Returns a representation of space that encloses this Boundable, preferably
   * not much bigger than this Boundable's boundary yet fast to test for intersection
   * with the bounds of other Boundables. The class of object returned depends
   * on the subclass of AbstractSTRtree.
   * @return an Envelope (for STRtrees), an Interval (for SIRtrees), or other object
   * (for other subclasses of AbstractSTRtree)
   * @see AbstractSTRtree.IntersectsOp
   */
	virtual void* getBounds()=0;
};

/**
 * Boundable wrapper for a non-Boundable spatial object. Used internally by
 * AbstractSTRtree.
 *
 */
class ItemBoundable: public Boundable {
private:
	void* bounds;
	void* item;
public:
	ItemBoundable(void* newBounds,void* newItem);
	void* getBounds();
	void* getItem();
};

/**
 * A contiguous portion of 1D-space. Used internally by SIRtree.
 * @see SIRtree
 *
 */
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
 * A node of the STR tree. The children of this node are either more nodes
 * (AbstractNodes) or real data (ItemBoundables). If this node contains real data
 * (rather than nodes), then we say that this node is a "leaf node".  
 *
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
	/**
	 * Returns a representation of space that encloses this Boundable,
	 * preferably not much bigger than this Boundable's boundary yet fast to
	 * test for intersection with the bounds of other Boundables. The class of
	 * object returned depends on the subclass of AbstractSTRtree.
	 * 
	 * @return an Envelope (for STRtrees), an Interval (for SIRtrees), or other
	 *         object (for other subclasses of AbstractSTRtree)
	 * @see AbstractSTRtree.IntersectsOp
	 */  
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
 * <p>
 * This implementation is based on Boundables rather than just AbstractNodes, 
 * because the STR algorithm operates on both nodes and 
 * data, both of which are treated here as Boundables.
 * 
 */
class AbstractSTRtree {
protected:
	/**
	* A test for intersection between two bounds, necessary because subclasses
	* of AbstractSTRtree have different implementations of bounds. 
	*/
	class IntersectsOp {
		public:
			/**
			* For STRtrees, the bounds will be Envelopes; for SIRtrees, Intervals;
			* for other subclasses of AbstractSTRtree, some other class.
			* @param aBounds the bounds of one spatial object
			* @param bBounds the bounds of another spatial object
			* @return whether the two bounds intersect
			*/
			virtual bool intersects(void* aBounds,void* bBounds)=0;
	};
	AbstractNode *root;
	virtual AbstractNode* createNode(int level)=0;
	virtual vector<Boundable*>* createParentBoundables(vector<Boundable*> *childBoundables,int newLevel);
	virtual AbstractNode* lastNode(vector<Boundable*> *nodes);
	virtual AbstractNode* getRoot();
	virtual void insert(void* bounds,void* item);
	virtual vector<void*>* query(void* searchBounds);
	/**
	* @return a test for intersection between two bounds, necessary because subclasses
	* of AbstractSTRtree have different implementations of bounds. 
	* @see IntersectsOp
	*/
//	virtual IntersectsOp* getIntersectsOp()=0;
	virtual vector<Boundable*>* boundablesAtLevel(int level);
	int nodeCapacity;
private:
	bool built;
	vector<Boundable*> *itemBoundables;
	virtual AbstractNode* createHigherLevels(vector<Boundable*> *boundablesOfALevel,int level);
	virtual vector<Boundable*> *sortBoundables(const vector<Boundable*> *input)=0;
public:
	IntersectsOp* intersectsOp;
	AbstractSTRtree(int newNodeCapacity);
	static bool compareDoubles(double a, double b);
	virtual ~AbstractSTRtree();
	virtual void build();
//	virtual void checkConsistency();
	virtual int getNodeCapacity();
	virtual void query(void* searchBounds,AbstractNode* node,vector<void*>* matches);
	virtual void boundablesAtLevel(int level,AbstractNode* top,vector<Boundable*> *boundables);
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
	vector<Boundable*> *sortBoundables(const vector<Boundable*> *input);
};
	
class STRAbstractNode: public AbstractNode{
public:
	STRAbstractNode(int level);
protected:
	void* computeBounds();
};

/**
 *  A query-only R-tree created using the Sort-Tile-Recursive (STR) algorithm. 
 *  For two-dimensional spatial data. <P>
 *
 *  The STR packed R-tree is simple to implement and maximizes space
 *  utilization; that is, as many leaves as possible are filled to capacity.
 *  Overlap between nodes is far less than in a basic R-tree. However, once the
 *  tree has been built (explicitly or on the first call to #query), items may
 *  not be added or removed. <P>
 * 
 * Described in: P. Rigaux, Michel Scholl and Agnes Voisard. Spatial Databases With
 *  Application To GIS. Morgan Kaufmann, San Francisco, 2002. 
 *
 */
class STRtree: public AbstractSTRtree,public SpatialIndex {
private:
//	Comparator* xComparator;
//	Comparator* yComparator;
//	IntersectsOp* intersectsOp;

	vector<Boundable*>* createParentBoundables(vector<Boundable*> *childBoundables, int newLevel);
	vector<Boundable*>* createParentBoundablesFromVerticalSlices(vector<vector<Boundable*>*>* verticalSlices, int newLevel);
protected:
	vector<Boundable*> *sortBoundables(const vector<Boundable*> *input);
	vector<Boundable*>* createParentBoundablesFromVerticalSlice(vector<Boundable*> *childBoundables, int newLevel);
	vector<vector<Boundable*>*>* verticalSlices(vector<Boundable*> *childBoundables, int sliceCount);
	AbstractNode* createNode(int level);
//	IntersectsOp* getIntersectsOp();
	class STRIntersectsOp:public AbstractSTRtree::IntersectsOp {
		public:
			bool intersects(void* aBounds,void* bBounds);
	};
public:
	STRtree();
	~STRtree();
	STRtree(int nodeCapacity);
	void insert(Envelope *itemEnv,void* item);
	vector<void*>* query(Envelope *searchEnv);
	static double centreX(Envelope *e);
	static double avg(double a, double b);
	static double centreY(Envelope *e);
};
}
#endif

