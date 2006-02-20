/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_INDEXSTRTREE_H
#define GEOS_INDEXSTRTREE_H

#include <memory>
#include <vector>
#include <geos/platform.h>
#include <geos/spatialIndex.h>
#include <geos/geom.h>


namespace geos {
namespace index { // geos.index

/// Contains 2-D and 1-D versions of the Sort-Tile-Recursive (STR) tree, a query-only R-tree.
namespace strtree { // geos.index.strtree

/*
 * \class Boundable indexStrtree.h geos/indexStrtree.h
 * \brief  A spatial object in an AbstractSTRtree.
 */
class Boundable {
public:
	/**
	 * Returns a representation of space that encloses this Boundable,
	 * preferably not much bigger than this Boundable's boundary yet
	 * fast to test for intersection with the bounds of other Boundables.
	 *
	 * The class of object returned depends
	 * on the subclass of AbstractSTRtree.
	 *
	 * @return an Envelope (for STRtrees), an Interval (for SIRtrees),
	 * or other object (for other subclasses of AbstractSTRtree)
	 *
	 * @see AbstractSTRtree::IntersectsOp
	 */
	virtual const void* getBounds()=0;
	virtual ~Boundable() {};
};

/*
 * \class ItemBoundable indexStrtree.h geos/indexStrtree.h
 *
 * \brief
 * Boundable wrapper for a non-Boundable spatial object.
 * Used internally by AbstractSTRtree.
 *
 */
class ItemBoundable: public Boundable {
private:
	const void* bounds;
	void* item;
public:
	ItemBoundable(const void* newBounds,void* newItem);
	virtual ~ItemBoundable();
	const void* getBounds();
	void* getItem();
};

/*
 * \class Interval indexStrtree.h geos/indexStrtree.h
 * \brief
 * A contiguous portion of 1D-space. Used internally by SIRtree.
 *
 * @see SIRtree
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

/*
 * \class AbstractNode indexStrtree.h geos/indexStrtree.h
 * \brief
 * A node of the STR tree.
 * 
 * The children of this node are either more nodes
 * (AbstractNodes) or real data (ItemBoundables).
 *
 * If this node contains real data (rather than nodes),
 * then we say that this node is a "leaf node".  
 *
 */
class AbstractNode: public Boundable {
private:
	std::vector<Boundable*> *childBoundables;
	int level;
public:
	AbstractNode(int newLevel, int capacity=10);
	virtual	~AbstractNode();
	inline std::vector<Boundable*>* getChildBoundables();

	/**
	 * Returns a representation of space that encloses this Boundable,
	 * preferably not much bigger than this Boundable's boundary yet fast to
	 * test for intersection with the bounds of other Boundables.
	 * The class of object returned depends on the subclass of
	 * AbstractSTRtree.
	 * 
	 * @return an Envelope (for STRtrees), an Interval (for SIRtrees),
	 *	or other object (for other subclasses of AbstractSTRtree)
	 *
	 * @see AbstractSTRtree::IntersectsOp
	 */  
	const void* getBounds();
	int getLevel();
	void addChildBoundable(Boundable *childBoundable);
protected:
	virtual void* computeBounds()=0;
	const void* bounds;
};

// INLINE funx 

inline std::vector<Boundable*>*
AbstractNode::getChildBoundables() {
	return childBoundables;
}

/*
 * \class AbstractSTRtree indexStrtree.h geos/indexStrtree.h
 *
 * \brief
 * Base class for STRtree and SIRtree.
 *
 * STR-packed R-trees are described in:
 * P. Rigaux, Michel Scholl and Agnes Voisard. Spatial Databases With
 * Application To GIS. Morgan Kaufmann, San Francisco, 2002.
 * 
 * This implementation is based on Boundables rather than just AbstractNodes, 
 * because the STR algorithm operates on both nodes and 
 * data, both of which are treated here as Boundables.
 * 
 */
class AbstractSTRtree {
protected:
	/*
	 * \class IntersectsOp indexStrtree.h geos/indexStrtree.h
	 *
	 * \brief
	 * A test for intersection between two bounds, necessary because
	 * subclasses of AbstractSTRtree have different implementations of
	 * bounds. 
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
			virtual bool intersects(const void* aBounds, const void* bBounds)=0;
	};
	AbstractNode *root;
	std::vector <AbstractNode *> *nodes;
	virtual AbstractNode* createNode(int level)=0;
	virtual std::vector<Boundable*>* createParentBoundables(std::vector<Boundable*> *childBoundables, int newLevel);
	virtual AbstractNode* lastNode(std::vector<Boundable*> *nodes);
	virtual AbstractNode* getRoot();
	virtual void insert(const void* bounds,void* item);
	virtual std::vector<void*>* query(const void* searchBounds);
	virtual std::vector<Boundable*>* boundablesAtLevel(int level);
	int nodeCapacity;

	/**
	 * @return a test for intersection between two bounds,
	 * necessary because subclasses
	 * of AbstractSTRtree have different implementations of bounds.
	 * @see IntersectsOp
	 */
	virtual IntersectsOp *getIntersectsOp()=0;
 
private:
	bool built;
	std::vector<Boundable*> *itemBoundables;
	virtual AbstractNode* createHigherLevels(std::vector<Boundable*> *boundablesOfALevel, int level);
	virtual std::vector<Boundable*> *sortBoundables(const std::vector<Boundable*> *input)=0;
public:
	AbstractSTRtree(int newNodeCapacity);
	static bool compareDoubles(double a, double b);
	virtual ~AbstractSTRtree();
	virtual void build();
//	virtual void checkConsistency();
	virtual int getNodeCapacity();
	virtual void query(const void* searchBounds, AbstractNode* node, std::vector<void*>* matches);
	virtual void boundablesAtLevel(int level,AbstractNode* top,std::vector<Boundable*> *boundables);
};

class SIRAbstractNode: public AbstractNode{
public:
	SIRAbstractNode(int level, int capacity);
	~SIRAbstractNode();
protected:
	void* computeBounds();
};

/*
 * \class SIRtree indexStrtree.h geos/indexStrtree.h
 * \brief One-dimensional version of an STR-packed R-tree.
 *
 * SIR stands for "Sort-Interval-Recursive".
 *
 * STR-packed R-trees are described in:
 * P. Rigaux, Michel Scholl and Agnes Voisard. Spatial Databases With
 * Application To GIS. Morgan Kaufmann, San Francisco, 2002.
 *
 * @see STRtree
 */
class SIRtree: public AbstractSTRtree {
using AbstractSTRtree::insert;
using AbstractSTRtree::query;

public:
	SIRtree();
	SIRtree(int nodeCapacity);
	virtual ~SIRtree();
	void insert(double x1,double x2,void* item);
	std::vector<void*>* query(double x);
	std::vector<void*>* query(double x1, double x2);

protected:
	class SIRIntersectsOp:public AbstractSTRtree::IntersectsOp {
		public:
			bool intersects(const void* aBounds, const void* bBounds);
	};
	std::vector<Boundable*>* createParentBoundables(std::vector<Boundable*> *childBoundables,int newLevel);
	AbstractNode* createNode(int level);
	IntersectsOp* getIntersectsOp() {return intersectsOp;};
	std::vector<Boundable*> *sortBoundables(const std::vector<Boundable*> *input);

private:
	IntersectsOp* intersectsOp;
};
	
class STRAbstractNode: public AbstractNode{
public:
	STRAbstractNode(int level, int capacity);
	~STRAbstractNode();
protected:
	void* computeBounds();
};

/*
 * \class STRtree indexStrtree.h geos/indexStrtree.h
 *
 * \brief
 * A query-only R-tree created using the Sort-Tile-Recursive (STR) algorithm. 
 * For two-dimensional spatial data. 
 *
 * The STR packed R-tree is simple to implement and maximizes space
 * utilization; that is, as many leaves as possible are filled to capacity.
 * Overlap between nodes is far less than in a basic R-tree. However, once the
 * tree has been built (explicitly or on the first call to #query), items may
 * not be added or removed. 
 * 
 * Described in: P. Rigaux, Michel Scholl and Agnes Voisard. Spatial
 * Databases With Application To GIS. Morgan Kaufmann, San Francisco, 2002. 
 *
 */
class STRtree: public AbstractSTRtree, public SpatialIndex
{
using AbstractSTRtree::insert;
using AbstractSTRtree::query;

private:
	std::vector<Boundable*>* createParentBoundables(std::vector<Boundable*> *childBoundables, int newLevel);
	std::vector<Boundable*>* createParentBoundablesFromVerticalSlices(std::vector<std::vector<Boundable*>*>* verticalSlices, int newLevel);

protected:
	std::vector<Boundable*> *sortBoundables(const std::vector<Boundable*> *input);
	std::vector<Boundable*>* createParentBoundablesFromVerticalSlice(std::vector<Boundable*> *childBoundables, int newLevel);
	std::vector<std::vector<Boundable*>*>* verticalSlices(std::vector<Boundable*> *childBoundables, int sliceCount);
	AbstractNode* createNode(int level);
	class STRIntersectsOp: public AbstractSTRtree::IntersectsOp {
		public:
			bool intersects(const void* aBounds, const void* bBounds);
	};

private:
	STRIntersectsOp intersectsOp;

protected:
	IntersectsOp* getIntersectsOp() {return (IntersectsOp *)&intersectsOp;};

public:
	~STRtree();
	STRtree(int nodeCapacity=10);
	void insert(const Envelope *itemEnv,void* item);
	std::vector<void*>* query(const Envelope *searchEnv);
	static double centreX(Envelope *e);
	inline static double avg(double a, double b);
	inline static double centreY(Envelope *e);
};
inline double STRtree::avg(double a, double b) { 
	return (a + b) / 2.0;
}
inline double STRtree::centreY(Envelope *e) {
	return STRtree::avg(e->getMinY(), e->getMaxY());
}


} // namespace geos.index.strtree
} // namespace geos.index
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.10  2005/11/10 15:20:32  strk
 * Made virtual overloads explicit.
 *
 * Revision 1.9  2005/02/22 15:16:30  strk
 * STRtree::avg() and STRtree::centreY() inlined.
 *
 * Revision 1.8  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.7  2004/11/08 15:58:13  strk
 * More performance tuning.
 *
 * Revision 1.6  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.5  2004/10/26 17:46:18  strk
 * Removed slash-stars in comments to remove annoying compiler warnings.
 *
 * Revision 1.4  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.3  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.2  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.14  2004/05/06 15:00:59  strk
 * Boundable destructor made virtual.
 * Added vector <AbstractNode *> *nodes member in AbstractSTRTree,
 * used to keep track of created node to cleanly delete them at
 * destruction time.
 *
 * Revision 1.13  2004/05/05 17:42:06  strk
 * AbstractNode destructor made virtual. AbstractNode::bounds made protected.
 * SIRAbstractNode and STRAbstractNode destructors added to get rid of
 * AbstractNode::bounds in the right way (is a void * casted to appropriate
 * Class in the subClasses).
 *
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
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

