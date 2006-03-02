/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geomgraph.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

namespace geos {
namespace geomgraph { // geos.geomgraph

NodeMap::NodeMap(const NodeFactory &newNodeFact):
	nodeFact(newNodeFact)
{
#if GEOS_DEBUG
	cerr<<"["<<this<<"] NodeMap::NodeMap"<<endl;
#endif
}

NodeMap::~NodeMap()
{
	NodeMap::const_iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
		delete it->second;
	}
}

Node*
NodeMap::addNode(const Coordinate& coord)
{
#if GEOS_DEBUG
        cerr<<"["<<this<<"] NodeMap::addNode("<<coord.toString()<<")";
#endif
        Node *node=find(coord);
        if (node==NULL) {
#if GEOS_DEBUG
                cerr<<" is new"<<endl;
#endif
                node=nodeFact.createNode(coord);
                nodeMap[const_cast<Coordinate *>(&coord)]=node;
        }
        else
        {
#if GEOS_DEBUG
                cerr<<" already found ("<<node->getCoordinate().toString()<<") - adding Z"<<endl;
#endif
                node->addZ(coord.z);
        }
        return node;
}

// first arg cannot be const because
// it is liable to label-merging ... --strk;
Node*
NodeMap::addNode(Node *n)
{
#if GEOS_DEBUG
	cerr<<"["<<this<<"] NodeMap::addNode("<<n->print()<<")";
#endif
	Coordinate *c=const_cast<Coordinate *>(&n->getCoordinate());
	Node *node=find(*c);
	if (node==NULL) {
#if GEOS_DEBUG
		cerr<<" is new"<<endl;
#endif
		nodeMap[c]=n;
		return n;
	}
#if GEOS_DEBUG
	else
	{
		cerr<<" found already, merging label"<<endl;
		const vector<double>&zvals = n->getZ();
		for (unsigned int i=0; i<zvals.size(); i++)
		{
			node->addZ(zvals[i]);
		}
	}
#endif // GEOS_DEBUG
	node->mergeLabel(n);
	return node;
}

void
NodeMap::add(EdgeEnd *e)
{
	Coordinate& p=e->getCoordinate();
	Node *n=addNode(p);
	n->add(e);
}

/*
 * @return the node if found; null otherwise
 */
Node*
NodeMap::find(const Coordinate& coord) const
{
	Coordinate *c=const_cast<Coordinate *>(&coord);

	NodeMap::const_iterator found=nodeMap.find(c);

	if (found==nodeMap.end())
		return NULL;
	else
		return found->second;
}

void
NodeMap::getBoundaryNodes(int geomIndex, vector<Node*>&bdyNodes) const
{
	NodeMap::const_iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
		Node *node=it->second;
		if (node->getLabel()->getLocation(geomIndex)==Location::BOUNDARY)
			bdyNodes.push_back(node);
	}
}

string
NodeMap::print() const
{
	string out="";
	NodeMap::const_iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++) {
		Node *node=it->second;
		out+=node->print();
	}
	return out;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
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
 * Revision 1.10  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.9  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.8  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.7  2005/11/09 13:44:28  strk
 * Cleanups in Node and NodeMap.
 * Optimization of EdgeIntersectionLessThen.
 *
 * Revision 1.6  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.5  2004/11/20 15:45:47  strk
 * Fixed Z merging in addNode(Node *)
 *
 * Revision 1.4  2004/11/20 15:41:41  strk
 * Added Z merging in ::addNode
 *
 * Revision 1.3  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

