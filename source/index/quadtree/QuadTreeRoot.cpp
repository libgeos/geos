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
 * Revision 1.8  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
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


#include <geos/indexQuadtree.h>
#include <geos/util.h>

namespace geos {

// the singleton root quad is centred at the origin.
Coordinate* QuadTreeRoot::origin=new Coordinate(0.0, 0.0);

QuadTreeRoot::QuadTreeRoot(){}
QuadTreeRoot::~QuadTreeRoot(){}

/**
* Insert an item into the quadtree this is the root of.
*/
void QuadTreeRoot::insert(const Envelope *itemEnv,void* item){
	int index=getSubnodeIndex(itemEnv,origin);
	// if index is -1, itemEnv must cross the X or Y axis.
	if (index==-1) {
		add(item);
		return;
	}

	/*
	 * the item must be contained in one quadrant, so insert it into the
	 * tree for that quadrant (which may not yet exist)
	 */
	QuadTreeNode *node=subnode[index];

	/*
	 *  If the subquad doesn't exist or this item is not contained in it,
	 *  have to expand the tree upward to contain the item.
	 */
	if (node==NULL || !node->getEnvelope()->contains(itemEnv)) {
		QuadTreeNode *largerNode=QuadTreeNode::createExpanded(node,itemEnv);
		//delete subnode[index];
		subnode[index]=largerNode;
	}

	/*
	 * At this point we have a subquad which exists and must contain
	 * contains the env for the item.  Insert the item into the tree.
	 */
	insertContained(subnode[index],itemEnv,item);

	//System.out.println("depth = " + root.depth() + " size = " + root.size());
	//System.out.println(" size = " + size());
}

/**
 * insert an item which is known to be contained in the tree rooted at
 * the given QuadNode root.  Lower levels of the tree will be created
 * if necessary to hold the item.
 */
void
QuadTreeRoot::insertContained(QuadTreeNode *tree, const Envelope *itemEnv, void *item)
{
	Assert::isTrue(tree->getEnvelope()->contains(itemEnv));
	/**
	* Do NOT create a new quad for zero-area envelopes - this would lead
	* to infinite recursion. Instead, use a heuristic of simply returning
	* the smallest existing quad containing the query
	*/
	bool isZeroX=IntervalSize::isZeroWidth(itemEnv->getMinX(),itemEnv->getMaxX());
	bool isZeroY=IntervalSize::isZeroWidth(itemEnv->getMinX(),itemEnv->getMaxX());
	QuadTreeNodeBase *node;
	if (isZeroX || isZeroY)
		node=tree->find(itemEnv);
	else
		node=tree->getNode(itemEnv);
	node->add(item);
}

bool QuadTreeRoot::isSearchMatch(const Envelope *searchEnv){
	return true;
}
}

