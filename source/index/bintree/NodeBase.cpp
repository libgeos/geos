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
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/indexBintree.h"
#include "../../headers/indexChain.h"

namespace geos {

/**
* Returns the index of the subnode that wholely contains the given interval.
* If none does, returns -1.
*/
int NodeBase::getSubnodeIndex(BinTreeInterval *interval, double centre) {
	int subnodeIndex=-1;
	if (interval->min>=centre) subnodeIndex=1;
	if (interval->max<=centre) subnodeIndex=0;
	return subnodeIndex;
}

NodeBase::NodeBase() {
	items=new vector<void*>();
	subnode[0]=NULL;
	subnode[1]=NULL;
}

NodeBase::~NodeBase() {
	for(int i=0;i<(int)items->size();i++) {
		delete (indexMonotoneChain*)(*items)[i];
	}
	delete items;
	delete subnode[0];
	delete subnode[1];
	subnode[0]=NULL;
	subnode[1]=NULL;
}

vector<void*>* NodeBase::getItems() {
	return items;
}

void NodeBase::add(void* item){
	items->push_back(item);
}

vector<void*>* NodeBase::addAllItems(vector<void*> *newItems) {
	items->insert(items->end(),newItems->begin(),newItems->end());
	for(int i=0;i<2;i++) {
		if (subnode[i]!=NULL) {
			subnode[i]->addAllItems(newItems);
		}
	}
	return items;
}

vector<void*>* NodeBase::addAllItemsFromOverlapping(BinTreeInterval *interval,vector<void*> *resultItems){
	if (!isSearchMatch(interval))
		return items;
	resultItems->insert(resultItems->end(),items->begin(),items->end());
	for (int i=0;i<2;i++) {
		if (subnode[i]!=NULL) {
			subnode[i]->addAllItemsFromOverlapping(interval,resultItems);
		}
	}
	return items;
}

int NodeBase::depth() {
	int maxSubDepth=0;
	for (int i=0;i<2;i++) {
		if (subnode[i]!=NULL) {
			int sqd=subnode[i]->depth();
			if (sqd>maxSubDepth)
				maxSubDepth=sqd;
		}
	}
	return maxSubDepth+1;
}

int NodeBase::size() {
	int subSize=0;
	for (int i=0;i<2;i++) {
		if (subnode[i]!=NULL) {
			subSize+=subnode[i]->size();
		}
	}
	return subSize+(int)items->size();
}

int NodeBase::nodeSize() {
	int subSize=0;
	for (int i=0;i<2;i++) {
		if (subnode[i]!=NULL) {
			subSize+=subnode[i]->nodeSize();
		}
	}
	return subSize+1;
}
}

