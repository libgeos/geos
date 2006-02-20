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

#include <sstream>
#include <geos/indexQuadtree.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

int
QuadTreeNodeBase::getSubnodeIndex(const Envelope *env, const Coordinate *centre)
{
	int subnodeIndex=-1;
	if (env->getMinX()>=centre->x) {
		if (env->getMinY()>=centre->y) subnodeIndex=3;
		if (env->getMaxY()<=centre->y) subnodeIndex=1;
	}
	if (env->getMaxX()<=centre->x) {
		if (env->getMinY()>=centre->y) subnodeIndex=2;
		if (env->getMaxY()<=centre->y) subnodeIndex=0;
	}
#if DEBUG
	cerr<<"getSubNodeIndex("<<env->toString()<<", "<<centre->toString()<<") returning "<<subnodeIndex<<endl;
#endif
	return subnodeIndex;
}

QuadTreeNodeBase::QuadTreeNodeBase() {
	items=new vector<void*>();
	subnode[0]=NULL;
	subnode[1]=NULL;
	subnode[2]=NULL;
	subnode[3]=NULL;
}

QuadTreeNodeBase::~QuadTreeNodeBase() {
	delete subnode[0];
	delete subnode[1];
	delete subnode[2];
	delete subnode[3];
	subnode[0]=NULL;
	subnode[1]=NULL;
	subnode[2]=NULL;
	subnode[3]=NULL;
	delete items;
}

vector<void*>* QuadTreeNodeBase::getItems() {
	return items;
}

void QuadTreeNodeBase::add(void* item) {
	items->push_back(item);
	//DEBUG itemCount++;
	//DEBUG System.out.print(itemCount);
}

vector<void*>*
QuadTreeNodeBase::addAllItems(vector<void*> *resultItems)
{
	//<<TODO:ASSERT?>> Can we assert that this node cannot have both items
	//and subnodes? [Jon Aquino]
	resultItems->insert(resultItems->end(),items->begin(),items->end());
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subnode[i]->addAllItems(resultItems);
		}
	}
	return resultItems;
}

void
QuadTreeNodeBase::addAllItemsFromOverlapping(const Envelope *searchEnv, vector<void*> *resultItems)
{
	if (!isSearchMatch(searchEnv))
		return;

	//<<TODO:ASSERT?>> Can we assert that this node cannot have both items
	//and subnodes? [Jon Aquino]
	resultItems->insert(resultItems->end(),items->begin(),items->end());
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subnode[i]->addAllItemsFromOverlapping(searchEnv,resultItems);
		}
	}
}

//<<TODO:RENAME?>> In Samet's terminology, I think what we're returning here is
//actually level+1 rather than depth. (See p. 4 of his book) [Jon Aquino]
int QuadTreeNodeBase::depth() {
	int maxSubDepth=0;
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			int sqd=subnode[i]->depth();
			if (sqd>maxSubDepth)
				maxSubDepth=sqd;
		}
	}
	return maxSubDepth+1;
}

//<<TODO:RENAME?>> "size" is a bit generic. How about "itemCount"? [Jon Aquino]
int QuadTreeNodeBase::size() {
	int subSize=0;
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subSize+=subnode[i]->size();
		}
	}
	return subSize+(int)items->size();
}

//<<TODO:RENAME?>> The Java Language Specification recommends that "Methods to
//get and set an attribute that might be thought of as a variable V should be
//named getV and setV" (6.8.3). Perhaps this and other methods should be
//renamed to "get..."? [Jon Aquino]
int QuadTreeNodeBase::nodeCount() {
	int subSize=0;
	for(int i=0;i<4;i++) {
		if (subnode[i]!=NULL) {
			subSize+=subnode[i]->size();
		}
	}
	return subSize+1;
}

string
QuadTreeNodeBase::toString() const
{
	ostringstream s;
	s<<"ITEMS:"<<items->size()<<endl;
	for (int i=0; i<4; i++)
	{
		s<<"subnode["<<i<<"] ";
		if ( subnode[i] == NULL ) s<<"NULL";
		else s<<subnode[i]->toString();
		s<<endl;
	}
	return s.str();
}

/*public*/
void
QuadTreeNodeBase::visit(const Envelope* searchEnv, ItemVisitor& visitor)
{
	if (! isSearchMatch(searchEnv)) return;

	// this node may have items as well as subnodes (since items may not
	// be wholely contained in any single subnode
	visitItems(searchEnv, visitor);

	for (int i = 0; i < 4; i++) {
		if (subnode[i] != NULL) {
			subnode[i]->visit(searchEnv, visitor);
		}
	}
}

/*private*/
void
QuadTreeNodeBase::visitItems(const Envelope* searchEnv, ItemVisitor& visitor)
{
	// would be nice to filter items based on search envelope, but can't
	// until they contain an envelope
	for (vector<void*>::iterator i=items->begin(), e=items->end();
			i!=e; i++)
	{
		visitor.visitItem(*i);
	}
}

/*public*/
bool
QuadTreeNodeBase::remove(const Envelope* itemEnv, void* item)
{
	// use envelope to restrict nodes scanned
	if (! isSearchMatch(itemEnv)) return false;

	bool found = false;
	for (int i = 0; i < 4; i++) {
		if (subnode[i] != NULL) {
			found = subnode[i]->remove(itemEnv, item);
			if (found) {
				// trim subtree if empty
				if (subnode[i]->isPrunable())
				subnode[i] = NULL;
				break;
			}
		}
	}
	// if item was found lower down, don't need to search for it here
	if (found) return found;

	// otherwise, try and remove the item from the list of items
	// in this node
	vector<void*>::iterator foundIter = 
		find(items->begin(), items->end(), item);
	if ( foundIter != items->end() ) {
		items->erase(foundIter);
		return true;
	} else {
		return false;
	}
}


} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.13  2006/02/20 21:04:37  strk
 * - namespace geos::index
 * - SpatialIndex interface synced
 *
 * Revision 1.12  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.11  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.10  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.9  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.8  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

