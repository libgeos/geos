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

#include <geos/index/quadtree/Node.h> 
#include <geos/index/quadtree/Key.h> 
#include <geos/geom/Envelope.h>

#include <string>
#include <sstream>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

Node*
Node::createNode(Envelope *env)
{
	Key* key=new Key(env);
	Node *node=new Node(new Envelope(*(key->getEnvelope())),key->getLevel());
	delete key;
	return node;
}

Node*
Node::createExpanded(Node *node, const Envelope *addEnv)
{
	Envelope *expandEnv=new Envelope(*addEnv);
	if (node!=NULL) expandEnv->expandToInclude(node->env);
#if GEOS_DEBUG
	cerr<<"Node::createExpanded computed "<<expandEnv->toString()<<endl;
#endif
	Node *largerNode=createNode(expandEnv);
	if (node!=NULL) largerNode->insertNode(node);
	delete expandEnv;
	return largerNode;
}

/*public*/
Node*
Node::getNode(const Envelope *searchEnv)
{
	int subnodeIndex=getSubnodeIndex(searchEnv, centre);
	// if subquadIndex is -1 searchEnv is not contained in a subquad
	if (subnodeIndex!=-1) {
		// create the quad if it does not exist
		Node *node=getSubnode(subnodeIndex);
		// recursively search the found/created quad
		return node->getNode(searchEnv);
	} else {
		return this;
	}
}

/*public*/
NodeBase*
Node::find(const Envelope *searchEnv)
{
	int subnodeIndex=getSubnodeIndex(searchEnv, centre);
	if (subnodeIndex==-1)
		return this;
	if (subnode[subnodeIndex]!=NULL) {
		// query lies in subquad, so search it
		Node *node=subnode[subnodeIndex];
		return node->find(searchEnv);
	}
	// no existing subquad, so return this one anyway
	return this;
}

void Node::insertNode(Node* node) {
	assert(env==NULL || env->contains(node->env));
	//System.out.println(env);
	//System.out.println(quad.env);
	int index=getSubnodeIndex(node->env, centre);
	//System.out.println(index);
	if (node->level==level-1) {
		subnode[index]=node;
		//System.out.println("inserted");
	} else {
		// the quad is not a direct child, so make a new child quad to contain it
		// and recursively insert the quad
		Node *childNode=createSubnode(index);
		childNode->insertNode(node);
		subnode[index]=childNode;
	}
}

/**
* get the subquad for the index.
* If it doesn't exist, create it
*/
Node* Node::getSubnode(int index){
	if (subnode[index]==NULL) {
		subnode[index]=createSubnode(index);
	}
	return subnode[index];
}

Node* Node::createSubnode(int index) {
	// create a new subquad in the appropriate quadrant
	double minx=0.0;
	double maxx=0.0;
	double miny=0.0;
	double maxy=0.0;

	switch (index) {
		case 0:
			minx=env->getMinX();
			maxx=centre.x;
			miny=env->getMinY();
			maxy=centre.y;
			break;
		case 1:
			minx=centre.x;
			maxx=env->getMaxX();
			miny=env->getMinY();
			maxy=centre.y;
			break;
	case 2:
			minx=env->getMinX();
			maxx=centre.x;
			miny=centre.y;
			maxy=env->getMaxY();
			break;
	case 3:
			minx=centre.x;
			maxx=env->getMaxX();
			miny=centre.y;
			maxy=env->getMaxY();
			break;
	}
	Envelope *sqEnv=new Envelope(minx,maxx,miny,maxy);
	Node *node=new Node(sqEnv,level-1);
	return node;
}

string
Node::toString() const
{
	ostringstream os;
	os <<"L"<<level<<" "<<env->toString()<<" Ctr["<<centre.toString()<<"]";
	os <<" "+NodeBase::toString();
	return os.str();
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 14:28:53  strk
 * Filenames renamed to match class names (matching JTS)
 *
 * Revision 1.17  2006/03/22 12:22:50  strk
 * indexQuadtree.h split
 *
 **********************************************************************/

