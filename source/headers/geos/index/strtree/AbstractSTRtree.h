/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_INDEX_STRTREE_ABSTRACTSTRTREE_H
#define GEOS_INDEX_STRTREE_ABSTRACTSTRTREE_H

#include <geos/index/strtree/AbstractNode.h> // for inlines

#include <vector>
#include <cassert> // for inlines

// Forward declarations
namespace geos {
	namespace index { 
		class ItemVisitor;
		namespace strtree { 
			class Boundable;
			class AbstractNode;
		}
	}
}

namespace geos {
namespace index { // geos::index
namespace strtree { // geos::index::strtree

/** \brief
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

private:
	bool built;
	std::vector<Boundable*> *itemBoundables;

	/**
	 * Creates the levels higher than the given level
	 * 
	 * @param boundablesOfALevel
	 *            the level to build on
	 * @param level
	 *            the level of the Boundables, or -1 if the boundables are item
	 *            boundables (that is, below level 0)
	 * @return the root, which may be a ParentNode or a LeafNode
	 */
	virtual AbstractNode* createHigherLevels(
			std::vector<Boundable*> *boundablesOfALevel,
			int level);

	virtual std::vector<Boundable*> *sortBoundables(
			const std::vector<Boundable*> *input)=0;

	bool remove(const void* searchBounds, AbstractNode& node, void* item);
	bool removeItem(AbstractNode& node, void* item);

protected:

	/** \brief
	 * A test for intersection between two bounds, necessary because
	 * subclasses of AbstractSTRtree have different implementations of
	 * bounds. 
	 */
	class IntersectsOp {
		public:
			/**
			 * For STRtrees, the bounds will be Envelopes; for
			 * SIRtrees, Intervals; for other subclasses of
			 * AbstractSTRtree, some other class.
			 * @param aBounds the bounds of one spatial object
			 * @param bBounds the bounds of another spatial object
			 * @return whether the two bounds intersect
			 */
			virtual bool intersects(const void* aBounds,
					const void* bBounds)=0;

			virtual ~IntersectsOp() {}
	};

	AbstractNode *root;

	std::vector <AbstractNode *> *nodes;

	virtual AbstractNode* createNode(int level)=0;

	/**
	 * Sorts the childBoundables then divides them into groups of size M, where
	 * M is the node capacity.
	 */
	virtual std::vector<Boundable*>* createParentBoundables(
			std::vector<Boundable*> *childBoundables,
			int newLevel);

	virtual AbstractNode* lastNode(std::vector<Boundable*> *nodes) {
		return static_cast<AbstractNode*>((*nodes)[nodes->size()-1]);
	}

	virtual AbstractNode* getRoot() {
		return root;
	}

	///  Also builds the tree, if necessary.
	virtual void insert(const void* bounds,void* item);

	///  Also builds the tree, if necessary.
	void query(const void* searchBounds, std::vector<void*>& foundItems);

#if 0
	///  Also builds the tree, if necessary.
	std::vector<void*>* query(const void* searchBounds) {
		vector<void*>* matches = new vector<void*>();
		query(searchBounds, *matches);
		return matches;
	}
#endif


	///  Also builds the tree, if necessary.
	void query(const void* searchBounds, ItemVisitor& visitor);

	void query(const void* searchBounds, AbstractNode& node, ItemVisitor& visitor);
  
	///  Also builds the tree, if necessary.
	bool remove(const void* itemEnv, void* item);


	virtual std::vector<Boundable*>* boundablesAtLevel(int level);

	// @@ should be size_t, probably
	size_t nodeCapacity;

	/**
	 * @return a test for intersection between two bounds,
	 * necessary because subclasses
	 * of AbstractSTRtree have different implementations of bounds.
	 * @see IntersectsOp
	 */
	virtual IntersectsOp *getIntersectsOp()=0;
 

public:

	/**
	 * Constructs an AbstractSTRtree with the specified maximum number of child
	 * nodes that a node may have
	 */
	AbstractSTRtree(size_t newNodeCapacity)
		:
		built(false),
			itemBoundables(new std::vector<Boundable*>()),
		nodes(new std::vector<AbstractNode *>()),
		nodeCapacity(newNodeCapacity)
	{
		assert(newNodeCapacity>1);
	}

	static bool compareDoubles(double a, double b) {
		return a<b;
	}

	virtual ~AbstractSTRtree();

	/**
	 * Creates parent nodes, grandparent nodes, and so forth up to the root
	 * node, for the data that has been inserted into the tree. Can only be
	 * called once, and thus can be called only after all of the data has been
	 * inserted into the tree.
	 */
	virtual void build();

	/**
	 * Returns the maximum number of child nodes that a node may have
	 */
	virtual size_t getNodeCapacity() { return nodeCapacity; }

	virtual void query(const void* searchBounds, AbstractNode* node, std::vector<void*>* matches);

	/**
	 * @param level -1 to get items
	 */
	virtual void boundablesAtLevel(int level, AbstractNode* top,
			std::vector<Boundable*> *boundables);
};


} // namespace geos::index::strtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_INDEX_STRTREE_ABSTRACTSTRTREE_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/06/12 10:49:43  strk
 * unsigned int => size_t
 *
 * Revision 1.2  2006/06/08 11:20:24  strk
 * Added missing virtual destructor to abstract classes.
 *
 * Revision 1.1  2006/03/21 10:47:34  strk
 * indexStrtree.h split
 *
 **********************************************************************/

