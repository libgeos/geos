/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/relate/EdgeEndBundleStar.java rev. 1.13 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/relate/EdgeEndBundleStar.h>
#include <geos/operation/relate/EdgeEndBundle.h>

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

void
EdgeEndBundleStar::updateIM(IntersectionMatrix& im)
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
 * Revision 1.13  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 * Revision 1.12  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.11  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 **********************************************************************/

