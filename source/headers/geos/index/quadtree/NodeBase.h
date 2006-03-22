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

#ifndef GEOS_IDX_QUADTREE_NODEBASE_H
#define GEOS_IDX_QUADTREE_NODEBASE_H

#include <vector>
#include <string>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
		class Envelope;
	}
	namespace index {
		class ItemVisitor;
		namespace quadtree {
			class Node;
		}
	}
}

namespace geos {
namespace index { // geos::index
namespace quadtree { // geos::index::quadtree

/**
 * \brief
 * The base class for nodes in a Quadtree.
 *
 */
class NodeBase {

private:

	void visitItems(const geom::Envelope* searchEnv,
			ItemVisitor& visitor);
	
public:

	static int getSubnodeIndex(const geom::Envelope *env,
			const geom::Coordinate& centre);

	NodeBase();

	virtual ~NodeBase();

	virtual std::vector<void*>* getItems();

	virtual void add(void* item);

	virtual std::vector<void*>* addAllItems(std::vector<void*> *resultItems);

	virtual void addAllItemsFromOverlapping(const geom::Envelope *searchEnv,
			std::vector<void*> *resultItems);

	virtual int depth();

	virtual int size();

	virtual int nodeCount();

	virtual std::string toString() const;

	virtual void visit(const geom::Envelope* searchEnv, ItemVisitor& visitor);

	/**
	 * Removes a single item from this subtree.
	 *
	 * @param searchEnv the envelope containing the item
	 * @param item the item to remove
	 * @return <code>true</code> if the item was found and removed
	 */
	bool remove(const geom::Envelope* itemEnv, void* item);
 
	bool hasItems() const;

	bool hasChildren() const;

	bool isPrunable() const;

protected:

	std::vector<void*> *items;

	/**
	 * subquads are numbered as follows:
	 * <pre>
	 *  2 | 3
	 *  --+--
	 *  0 | 1
	 * </pre>
	 */
	Node* subnode[4];

	virtual bool isSearchMatch(const geom::Envelope *searchEnv)=0;
};


// INLINES, To be moved in NodeBase.inl

inline bool
NodeBase::hasChildren() const
{
	for (int i = 0; i < 4; i++) 
		if (subnode[i]) return true;
	return false;
}

inline bool
NodeBase::isPrunable() const
{
	return ! (hasChildren() || hasItems());
}

inline bool
NodeBase::hasItems() const
{
	return ! items->empty();
}

} // namespace geos::index::quadtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_IDX_QUADTREE_NODEBASE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/

