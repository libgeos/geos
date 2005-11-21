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

namespace geos {

EdgeEndBundleStar::EdgeEndBundleStar():
	EdgeEndStar()
{
}

EdgeEndBundleStar::~EdgeEndBundleStar()
{
	set<EdgeEnd*,EdgeEndLT>::iterator it=edgeMap->begin();
	for (;it!=edgeMap->end(); ++it)
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
	set<EdgeEnd*,EdgeEndLT>::iterator i=edgeMap->find(e);
	if (i==edgeMap->end())
	{
		eb=new EdgeEndBundle(e);
		insertEdgeEnd(eb);
	}
	else
	{
		eb=static_cast<EdgeEndBundle*>(*i);
		eb->insert(e);
	}
}

/**
 * Update the IM with the contribution for the EdgeStubs around the node.
 */
void
EdgeEndBundleStar::updateIM(IntersectionMatrix *im)
{
	vector<EdgeEnd*> *edges=getEdges();
	for (vector<EdgeEnd*>::iterator it=edges->begin();it<edges->end(); ++it)
	{
		EdgeEndBundle *esb=static_cast<EdgeEndBundle*>(*it);
		esb->updateIM(im);
	}
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.9  2005/11/21 16:03:20  strk
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

