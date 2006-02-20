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
#include <geos/util.h>

namespace geos {
namespace index { // geos.index
namespace bintree { // geos.index.bintree

BinTreeNode* BinTreeNode::createNode(BinTreeInterval *itemInterval){
	Key *key=new Key(itemInterval);
	//System.out.println("input: " + env + "  binaryEnv: " + key.getEnvelope());
	BinTreeNode* node=new BinTreeNode(new BinTreeInterval(key->getInterval()),key->getLevel());
	delete key;
	return node;
}

BinTreeNode* BinTreeNode::createExpanded(BinTreeNode *node,BinTreeInterval *addInterval){
	BinTreeInterval *expandInt=new BinTreeInterval(addInterval);
	if (node!=NULL) expandInt->expandToInclude(node->interval);
	BinTreeNode *largerNode=createNode(expandInt);
	if (node!=NULL) largerNode->insert(node);
	delete expandInt;
	return largerNode;
}

BinTreeNode::BinTreeNode(BinTreeInterval *newInterval,int newLevel){
	interval=newInterval;
	level=newLevel;
	centre=(interval->getMin()+interval->getMax())/2;
}

BinTreeNode::~BinTreeNode(){
	delete interval;
}

BinTreeInterval* BinTreeNode::getInterval() {
	return interval;
}

bool BinTreeNode::isSearchMatch(BinTreeInterval *itemInterval){
	return itemInterval->overlaps(interval);
}

/**
* Returns the subnode containing the envelope.
* Creates the node if
* it does not already exist.
*/
BinTreeNode* BinTreeNode::getNode(BinTreeInterval *searchInterval){
	int subnodeIndex=getSubnodeIndex(searchInterval,centre);
	// if index is -1 searchEnv is not contained in a subnode
	if (subnodeIndex!=-1) {
		// create the node if it does not exist
		BinTreeNode* node=getSubnode(subnodeIndex);
		// recursively search the found/created node
		return node->getNode(searchInterval);
	} else {
		return this;
	}
}

/**
* Returns the smallest <i>existing</i>
* node containing the envelope.
*/
NodeBase* BinTreeNode::find(BinTreeInterval *searchInterval){
	int subnodeIndex=getSubnodeIndex(searchInterval,centre);
	if (subnodeIndex==-1)
		return this;
	if (subnode[subnodeIndex]!=NULL) {
		// query lies in subnode, so search it
		BinTreeNode *node=subnode[subnodeIndex];
		return node->find(searchInterval);
	}
	// no existing subnode, so return this one anyway
	return this;
}

void BinTreeNode::insert(BinTreeNode *node) {
	Assert::isTrue(interval==NULL || interval->contains(node->interval));
	int index=getSubnodeIndex(node->interval,centre);
	if (node->level==level-1) {
		subnode[index]=node;
	} else {
		// the node is not a direct child, so make a new child node to contain it
		// and recursively insert the node
		BinTreeNode* childNode=createSubnode(index);
		childNode->insert(node);
		subnode[index]=childNode;
	}
}

/**
* get the subnode for the index.
* If it doesn't exist, create it
*/
BinTreeNode* BinTreeNode::getSubnode(int index){
	if (subnode[index]==NULL) {
		subnode[index]=createSubnode(index);
	}
	return subnode[index];
}

BinTreeNode* BinTreeNode::createSubnode(int index) {
	// create a new subnode in the appropriate interval
	double min=0.0;
	double max=0.0;
	switch (index) {
		case 0:
			min=interval->getMin();
			max=centre;
			break;
		case 1:
			min=centre;
			max=interval->getMax();
			break;
	}
	BinTreeInterval* subInt=new BinTreeInterval(min,max);
	BinTreeNode *node=new BinTreeNode(subInt,level-1);
	return node;
}

} // namespace geos.index.bintree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.6  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

