/**********************************************************************
 * $Id: Quadtree.cpp 2410 2009-04-27 15:30:52Z strk $
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
 * Last port: index/quadtree/Quadtree.java rev. 1.16 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/index/quadtree/Quadtree.h>
#include <geos/geom/Envelope.h>

#include <vector>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

/*public static*/
Envelope*
Quadtree::ensureExtent(const Envelope *itemEnv, double minExtent)
{
	//The names "ensureExtent" and "minExtent" are misleading -- sounds like
	//this method ensures that the extents are greater than minExtent.
	//Perhaps we should rename them to "ensurePositiveExtent" and "defaultExtent".
	//[Jon Aquino]
	double minx=itemEnv->getMinX();
	double maxx=itemEnv->getMaxX();
	double miny=itemEnv->getMinY();
	double maxy=itemEnv->getMaxY();
	// has a non-zero extent
	if (minx!=maxx && miny!=maxy) return (Envelope *)itemEnv;
	// pad one or both extents
	if (minx==maxx) {
		minx=minx-minExtent/2.0;
		maxx=minx+minExtent/2.0;
	}
	if (miny==maxy) {
		miny=miny-minExtent/2.0;
		maxy=miny+minExtent/2.0;
	}
	Envelope *newEnv = new Envelope(minx, maxx, miny, maxy);
	return newEnv;
}

/*public*/
Quadtree::~Quadtree()
{
	for (unsigned int i=0; i<newEnvelopes.size(); i++)
		delete newEnvelopes[i];
}

/*public*/
int
Quadtree::depth()
{
	return root.depth();
}

/*public*/
int
Quadtree::size()
{
	return root.size();
}

/*public*/
void
Quadtree::insert(const Envelope *itemEnv, void* item)
{
	collectStats(*itemEnv);

	Envelope *insertEnv=ensureExtent(itemEnv,minExtent);
	if ( insertEnv != itemEnv ) newEnvelopes.push_back(insertEnv);
	root.insert(insertEnv,item);
#if GEOS_DEBUG
	cerr<<"Quadtree::insert("<<itemEnv->toString()<<", "<<item<<")"<<endl;
	cerr<<"       insertEnv:"<<insertEnv->toString()<<endl;
	cerr<<"       tree:"<<endl<<root.toString()<<endl;
#endif
}


/*public*/
void
Quadtree::query(const Envelope *searchEnv,
	vector<void*>& foundItems)
{
	/*
	 * the items that are matched are the items in quads which
	 * overlap the search envelope
	 */
	root.addAllItemsFromOverlapping(*searchEnv, foundItems);
#if GEOS_DEBUG
	cerr<<"Quadtree::query returning "<<foundItems.size()
		<<" items over "<<size()
		<<" items in index (of depth: "<<depth()<<")"<<endl;
	cerr<<" Root:\n"<<root.toString()<<endl;
#endif
}

/*public*/
vector<void*>*
Quadtree::queryAll()
{
	vector<void*> *foundItems=new vector<void*>();
	root.addAllItems(*foundItems);
	return foundItems;
}

/*public*/
bool
Quadtree::remove(const Envelope* itemEnv, void* item)
{
	Envelope* posEnv = ensureExtent(itemEnv, minExtent);
	bool ret = root.remove(posEnv, item);
	if ( posEnv != itemEnv ) delete posEnv;
	return ret;
}

/*private*/
void
Quadtree::collectStats(const Envelope& itemEnv)
{
	double delX = itemEnv.getWidth();
	if (delX < minExtent && delX > 0.0)
		minExtent = delX;

	double delY = itemEnv.getHeight();
	if (delY < minExtent && delY > 0.0)
		minExtent = delY;
}

/*public*/
string
Quadtree::toString() const
{
	string ret = root.toString();
	return ret;
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.25  2006/04/03 08:29:30  strk
 * Added port info, cleaned up log message, minor assertion checking.
 *
 * Revision 1.24  2006/03/23 13:31:58  strk
 * Fixed to allow build with GEOS_DEBUG
 *
 * Revision 1.23  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 * Revision 1.22  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.21  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.20  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.19  2006/03/01 13:49:45  strk
 * Fixed debug lines (bug#36)
 *
 **********************************************************************/

