/**********************************************************************
 * $Id: Root.cpp 3274 2011-03-26 14:02:20Z strk $
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

#include <geos/index/bintree/Root.h>
#include <geos/index/bintree/Node.h>
#include <geos/index/bintree/Interval.h>
#include <geos/index/quadtree/IntervalSize.h>

#include <cstddef>
#include <cassert>

namespace geos {
namespace index { // geos.index
namespace bintree { // geos.index.bintree

double Root::origin=0.0;

void
Root::insert(Interval *itemInterval,void* item)
{
	int index=getSubnodeIndex(itemInterval,origin);
	// if index is -1, itemEnv must contain the origin.
	if (index==-1) {
		add(item);
		return;
	}

	/**
	 * the item must be contained in one interval, so insert it into the
	 * tree for that interval (which may not yet exist)
	 */
	Node *node=subnode[index];

	/**
	 *  If the subnode doesn't exist or this item is not contained in it,
	 *  have to expand the tree upward to contain the item.
	 */
	if (node==NULL || !node->getInterval()->contains(itemInterval))
	{
		Node* largerNode=Node::createExpanded(node,itemInterval);
//		delete subnode[index];
		subnode[index]=largerNode;
	}
	/**
	* At this point we have a subnode which exists and must contain
	* contains the env for the item.  Insert the item into the tree.
	*/
	insertContained(subnode[index],itemInterval,item);
	//System.out.println("depth = " + root.depth() + " size = " + root.size());
}

void
Root::insertContained(Node *tree, Interval *itemInterval, void* item)
{
	using geos::index::quadtree::IntervalSize;  

	assert(tree->getInterval()->contains(itemInterval));

	/**
	 * Do NOT create a new node for zero-area intervals - this would lead
	 * to infinite recursion. Instead, use a heuristic of simply returning
	 * the smallest existing node containing the query
	 */
	bool isZeroArea=IntervalSize::isZeroWidth(itemInterval->getMin(),
			itemInterval->getMax());
	NodeBase *node;
	if (isZeroArea)
		node=tree->find(itemInterval);
	else
		node=tree->getNode(itemInterval);
	node->add(item);
}

} // namespace geos.index.bintree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/22 16:01:33  strk
 * indexBintree.h header split, classes renamed to match JTS
 *
 * Revision 1.9  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 * Revision 1.8  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.7  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

