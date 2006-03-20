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

#include <cassert>

#include <geos/indexQuadtree.h>
//#include <geos/util.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>

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
//Coordinate* QuadTreeRoot::origin=new Coordinate(0.0, 0.0);
Coordinate QuadTreeRoot::origin(0.0, 0.0);

/*public*/
void
QuadTreeRoot::insert(const Envelope *itemEnv, void* item)
{

#if GEOS_DEBUG
	std::cerr<<"("<<this<<") insert("<<itemEnv->toString()<<", "<<item<<") called"<<std::endl;
#endif
	int index=getSubnodeIndex(itemEnv,origin);
	// if index is -1, itemEnv must cross the X or Y axis.
	if (index==-1) {
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
	QuadTreeNode *node=subnode[index];

#if GEOS_DEBUG
	std::cerr<<"("<<this<<") subnode["<<index<<"] @ "<<node<<std::endl;
#endif

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

/*private*/
void
QuadTreeRoot::insertContained(QuadTreeNode *tree, const Envelope *itemEnv, void *item)
{
	assert(tree->getEnvelope()->contains(itemEnv));

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

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.14  2006/03/09 15:36:25  strk
 * Fixed debugging lines
 *
 * Revision 1.13  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.12  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.11  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.10  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.9  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
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

