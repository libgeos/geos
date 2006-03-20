/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/opRelate.h>

using namespace std;
using namespace geos::geomgraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace relate { // geos.operation.relate

EdgeEndBundleStar::~EdgeEndBundleStar()
{
	EdgeEndStar::iterator it=begin();
	EdgeEndStar::iterator endIt=end();
	for (; it!=endIt; ++it)
	{
		EdgeEndBundle *eeb=static_cast<EdgeEndBundle*>(*it);
		delete eeb;
	}
}

/**
 * Insert a EdgeEnd in order in the list.
 * If there is an existing EdgeStubBundle which is parallel, the EdgeEnd is
 * added to the bundle.  Otherwise, a new EdgeEndBundle is created
 * to contain the EdgeEnd.
 */
void
EdgeEndBundleStar::insert(EdgeEnd *e)
{
	EdgeEndBundle *eb;
	//set<EdgeEnd*,EdgeEndLT>::iterator i=edgeMap->find(e);
	EdgeEndStar::iterator it=find(e);
	if (it==end())
	{
		eb=new EdgeEndBundle(e);
		insertEdgeEnd(eb);
	}
	else
	{
		eb=static_cast<EdgeEndBundle*>(*it);
		eb->insert(e);
	}
}

/**
 * Update the IM with the contribution for the EdgeStubs around the node.
 */
void
EdgeEndBundleStar::updateIM(IntersectionMatrix *im)
{
	//vector<EdgeEnd*> *edges=getEdges();
	EdgeEndStar::iterator it=begin();
	EdgeEndStar::iterator endIt=end();
	for (; it!=endIt; ++it)
	{
		EdgeEndBundle *esb=static_cast<EdgeEndBundle*>(*it);
		esb->updateIM(im);
	}
}

} // namespace geos.operation.relate
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.12  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.11  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.10  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.9  2005/11/21 16:03:20  strk
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
 * Revision 1.8  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

