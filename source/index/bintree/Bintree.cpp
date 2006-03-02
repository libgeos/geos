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

#include <geos/indexBintree.h>

namespace geos {
namespace index { // geos.index
namespace bintree { // geos.index.bintree

/**
* Ensure that the BinTreeInterval for the inserted item has non-zero extents.
* Use the current minExtent to pad it, if necessary
*/
BinTreeInterval* Bintree::ensureExtent(BinTreeInterval *itemInterval,double minExtent){
	double min=itemInterval->getMin();
	double max=itemInterval->getMax();
	// has a non-zero extent
	if (min!=max) return new BinTreeInterval(itemInterval);
	// pad extent
	if (min==max) {
		min=min-minExtent/2.0;
		max=min+minExtent/2.0;
	}
//	delete itemInterval;
	return new BinTreeInterval(min, max);
}



Bintree::Bintree() {
	minExtent=1.0;
	root=new Root();
}

Bintree::~Bintree() {
	for (unsigned int i=0; i<newIntervals.size(); i++)
		delete newIntervals[i];
	delete root;
}

int Bintree::depth(){
	if (root!=NULL) return root->depth();
	return 0;
}

int Bintree::size() {
	if (root!=NULL) return root->size();
	return 0;
}

/**
* Compute the total number of nodes in the tree
*
* @return the number of nodes in the tree
*/
int Bintree::nodeSize(){
	if (root!=NULL) return root->nodeSize();
	return 0;
}

void Bintree::insert(BinTreeInterval *itemInterval,void* item){
	collectStats(itemInterval);
	BinTreeInterval *insertInterval=ensureExtent(itemInterval,minExtent);
	if ( insertInterval != itemInterval )
		newIntervals.push_back(insertInterval);
	//int oldSize=size();
	root->insert(insertInterval,item);
	/* GEOS_DEBUG
	int newSize=size();
	System.out.println("BinTree: size="+newSize+"   node size="+nodeSize());
	if (newSize <= oldSize) {
	System.out.println("Lost item!");
	root.insert(insertInterval, item);
	System.out.println("reinsertion size="+size());
	}
	*/
}

vector<void*>* Bintree::iterator() {
	vector<void*>* foundItems=new vector<void*>();
	root->addAllItems(foundItems);
	return foundItems;
}

vector<void*>* Bintree::query(double x) {
	return query(new BinTreeInterval(x, x));
}

/**
* min and max may be the same value
*/
vector<void*>* Bintree::query(BinTreeInterval *interval) {
	/**
	* the items that are matched are all items in intervals
	* which overlap the query interval
	*/
	vector<void*>* foundItems=new vector<void*>();
	query(interval,foundItems);
	return foundItems;
}

void Bintree::query(BinTreeInterval *interval,vector<void*> *foundItems) {
	root->addAllItemsFromOverlapping(interval,foundItems);
}

void Bintree::collectStats(BinTreeInterval *interval) {
	double del=interval->getWidth();
	if (del<minExtent && del>0.0)
		minExtent=del;
}

} // namespace geos.index.bintree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.10  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.9  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.8  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/05/07 14:13:02  strk
 * Fixed segfault in ::insert
 *
 * Revision 1.6  2004/05/06 16:30:58  strk
 * Kept track of newly allocated objects by ensureExtent for Bintree and Quadtree,
 * deleted at destruction time. doc/example.cpp runs with no leaks.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

