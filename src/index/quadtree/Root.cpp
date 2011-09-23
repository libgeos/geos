/**********************************************************************
 * $Id: Root.cpp 2415 2009-04-27 19:38:44Z strk $
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
 **********************************************************************
 *
 * Last port: index/quadtree/Root.java rev 1.7 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/index/quadtree/Root.h>
#include <geos/index/quadtree/Node.h>
#include <geos/index/quadtree/IntervalSize.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>

#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

// the singleton root quad is centred at the origin.
//Coordinate* Root::origin=new Coordinate(0.0, 0.0);
const Coordinate Root::origin(0.0, 0.0);

/*public*/
void
Root::insert(const Envelope *itemEnv, void* item)
{

#if GEOS_DEBUG
	std::cerr<< "Root("<<this<<")::insert("<<itemEnv->toString()<<", "<<item<<") called"<<std::endl;
#endif
	int index = getSubnodeIndex(itemEnv, origin);
	// if index is -1, itemEnv must cross the X or Y axis.
	if (index==-1)
	{
#if GEOS_DEBUG
	std::cerr<<"  -1 subnode index"<<std::endl;
#endif
		add(item);
		return;
	}

	/*
	 * the item must be contained in one quadrant, so insert it into the
	 * tree for that quadrant (which may not yet exist)
	 */
	Node *node = subnode[index];

#if GEOS_DEBUG
	std::cerr<<"("<<this<<") subnode["<<index<<"] @ "<<node<<std::endl;
#endif

	/*
	 *  If the subquad doesn't exist or this item is not contained in it,
	 *  have to expand the tree upward to contain the item.
	 */
	if (node==NULL || !node->getEnvelope()->contains(itemEnv))
	{
		std::auto_ptr<Node> snode (node); // may be NULL
		node = 0; subnode[index] = 0;

		std::auto_ptr<Node> largerNode =
			Node::createExpanded(snode, *itemEnv);

#if GEOS_DEBUG
		std::cerr<<"("<<this<<") created expanded node " << largerNode.get() << " containing previously reported subnode" << std::endl;
#endif

		// Previous subnode was passed as a child of the larger one
		assert(!subnode[index]);
		subnode[index] = largerNode.release();
	}

#if GEOS_DEBUG
	std::cerr<<"("<<this<<") calling insertContained with subnode " << subnode[index] << std::endl;
#endif
	/*
	 * At this point we have a subquad which exists and must contain
	 * contains the env for the item.  Insert the item into the tree.
	 */
	insertContained(subnode[index], itemEnv, item);

#if GEOS_DEBUG
	std::cerr<<"("<<this<<") done calling insertContained with subnode " << subnode[index] << std::endl;
#endif

	//System.out.println("depth = " + root.depth() + " size = " + root.size());
	//System.out.println(" size = " + size());
}

/*private*/
void
Root::insertContained(Node *tree, const Envelope *itemEnv, void *item)
{
	assert(tree->getEnvelope()->contains(itemEnv));

	/**
	 * Do NOT create a new quad for zero-area envelopes - this would lead
	 * to infinite recursion. Instead, use a heuristic of simply returning
	 * the smallest existing quad containing the query
	 */
	bool isZeroX = IntervalSize::isZeroWidth(itemEnv->getMinX(),
	                                         itemEnv->getMaxX());
	bool isZeroY = IntervalSize::isZeroWidth(itemEnv->getMinY(),
	                                         itemEnv->getMaxY());

	NodeBase *node;

	if (isZeroX || isZeroY)
	{
		node = tree->find(itemEnv);
	}
	else
	{
		node = tree->getNode(itemEnv);
	}

	node->add(item);
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 14:28:53  strk
 * Filenames renamed to match class names (matching JTS)
 *
 * Revision 1.16  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/

