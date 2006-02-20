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

#include <geos/indexQuadtree.h>
#include <geos/util.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

Envelope*
Quadtree::ensureExtent(const Envelope *itemEnv,double minExtent)
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

Quadtree::~Quadtree()
{
	for (unsigned int i=0; i<newEnvelopes.size(); i++)
		delete newEnvelopes[i];
	delete root;
}

int
Quadtree::depth()
{
	//I don't think it's possible for root to be null. Perhaps we should
	//remove the check. [Jon Aquino]
    //Or make an assertion [Jon Aquino 10/29/2003] 
	if (root!=NULL) return root->depth();
	return 0;
}

int
Quadtree::size()
{
	if (root!=NULL) return root->size();
	return 0;
}

void
Quadtree::insert(const Envelope *itemEnv, void* item)
{
	collectStats(itemEnv);
	Envelope *insertEnv=ensureExtent(itemEnv,minExtent);
	if ( insertEnv != itemEnv ) newEnvelopes.push_back(insertEnv);
	root->insert(insertEnv,item);
#if DEBUG
	cerr<<"Quadtree::insert("<<itemEnv->toString()<<", "<<item<<")"<<endl;
	cerr<<"       insertEnv:"<<insertEnv->toString()<<endl;
	cerr<<"       tree:"<<endl<<root->toString()<<endl;
#endif
}


vector<void*>*
Quadtree::query(const Envelope *searchEnv)
{
	/*
	 * the items that are matched are the items in quads which
	 * overlap the search envelope
	 */
	vector<void*> *foundItems=new vector<void*>();
	root->addAllItemsFromOverlapping(searchEnv,foundItems);
#if DEBUG
	cerr<<"Quadtree::query returning "<<foundItems->size()<<" items over "<<size()<<" items in index (of depth: "<<depth()<<")"<<endl;
	cerr<<" Root:\n"<<root->toString()<<endl;
#endif
	return foundItems;
}


/*public*/
vector<void*>*
Quadtree::queryAll()
{
	vector<void*> *foundItems=new vector<void*>();
	root->addAllItems(foundItems);
	return foundItems;
}

/*public*/
bool
Quadtree::remove(const Envelope* itemEnv, void* item)
{
	Envelope* posEnv = ensureExtent(itemEnv, minExtent);
	return root->remove(posEnv, item);
}

void
Quadtree::collectStats(const Envelope *itemEnv)
{
	double delX=itemEnv->getWidth();
	if (delX<minExtent && delX>0.0)
		minExtent=delX;
	double delY=itemEnv->getWidth();
	if (delY<minExtent && delY>0.0)
		minExtent=delY;
}

string
Quadtree::toString() const
{
	string ret = root->toString();
	return ret;
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.17  2006/02/20 21:04:37  strk
 * - namespace geos::index
 * - SpatialIndex interface synced
 *
 * Revision 1.16  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.15  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.14  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.13  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.12  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.11  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/05/06 16:30:58  strk
 * Kept track of newly allocated objects by ensureExtent for Bintree and Quadtree,
 * deleted at destruction time. doc/example.cpp runs with no leaks.
 *
 * Revision 1.9  2004/04/19 15:14:45  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.8  2004/03/25 02:23:55  ybychkov
 * All "index/" packages upgraded to JTS 1.4
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

