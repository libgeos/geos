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
 * Revision 1.8  2004/03/25 02:23:55  ybychkov
 * All "index/*" packages upgraded to JTS 1.4
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/indexQuadtree.h"
#include "../../headers/util.h"

namespace geos {

/**
* Ensure that the envelope for the inserted item has non-zero extents.
* Use the current minExtent to pad the envelope, if necessary
*/
Envelope* Quadtree::ensureExtent(Envelope *itemEnv,double minExtent) {
	//The names "ensureExtent" and "minExtent" are misleading -- sounds like
	//this method ensures that the extents are greater than minExtent.
	//Perhaps we should rename them to "ensurePositiveExtent" and "defaultExtent".
	//[Jon Aquino]
	double minx=itemEnv->getMinX();
	double maxx=itemEnv->getMaxX();
	double miny=itemEnv->getMinY();
	double maxy=itemEnv->getMaxY();
	// has a non-zero extent
	if (minx!=maxx && miny!=maxy) return itemEnv;
	// pad one or both extents
	if (minx==maxx) {
		minx=minx-minExtent/2.0;
		maxx=minx+minExtent/2.0;
	}
	if (miny==maxy) {
		miny=miny-minExtent/2.0;
		maxy=miny+minExtent/2.0;
	}
	return new Envelope(minx, maxx, miny, maxy);
}

/**
* Constructs a Quadtree with zero items.
*/
Quadtree::Quadtree(){
	minExtent=1.0;
	root=new QuadTreeRoot();
}

Quadtree::~Quadtree(){
	delete root;
}

/**
* Returns the number of levels in the tree.
*/
int Quadtree::depth() {
	//I don't think it's possible for root to be null. Perhaps we should
	//remove the check. [Jon Aquino]
    //Or make an assertion [Jon Aquino 10/29/2003] 
	if (root!=NULL) return root->depth();
	return 0;
}

/**
* Returns the number of items in the tree.
*/
int Quadtree::size() {
	if (root!=NULL) return root->size();
	return 0;
}

void Quadtree::insert(Envelope *itemEnv,void* item){
	collectStats(itemEnv);
	Envelope *insertEnv=ensureExtent(itemEnv,minExtent);
	root->insert(insertEnv,item);
}


vector<void*>* Quadtree::query(Envelope *searchEnv){
	/**
	* the items that are matched are the items in quads which
	* overlap the search envelope
	*/
	vector<void*> *foundItems=new vector<void*>();
	root->addAllItemsFromOverlapping(searchEnv,foundItems);
	return foundItems;
}

/**
* Return a list of all items in the Quadtree
*/
vector<void*>* Quadtree::queryAll() {
	vector<void*> *foundItems=new vector<void*>();
	root->addAllItems(foundItems);
	return foundItems;
}

void Quadtree::collectStats(Envelope *itemEnv){
	double delX=itemEnv->getWidth();
	if (delX<minExtent && delX>0.0)
		minExtent=delX;
	double delY=itemEnv->getWidth();
	if (delY<minExtent && delY>0.0)
		minExtent=delY;
}
}

