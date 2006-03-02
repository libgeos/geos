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
#include <geos/util.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

namespace geos {
namespace index { // geos.index
namespace quadtree { // geos.index.quadtree

QuadTreeNode* QuadTreeNode::createNode(Envelope *env) {
	QuadTreeKey* key=new QuadTreeKey(env);
	QuadTreeNode *node=new QuadTreeNode(new Envelope(*(key->getEnvelope())),key->getLevel());
	delete key;
	return node;
}

QuadTreeNode*
QuadTreeNode::createExpanded(QuadTreeNode *node, const Envelope *addEnv)
{
	Envelope *expandEnv=new Envelope(*addEnv);
	if (node!=NULL) expandEnv->expandToInclude(node->env);
#if GEOS_DEBUG
	cerr<<"QuadTreeNode::createExpanded computed "<<expandEnv->toString()<<endl;
#endif
	QuadTreeNode *largerNode=createNode(expandEnv);
	if (node!=NULL) largerNode->insertNode(node);
	delete expandEnv;
	return largerNode;
}

/*public*/
QuadTreeNode*
QuadTreeNode::getNode(const Envelope *searchEnv)
{
	int subnodeIndex=getSubnodeIndex(searchEnv, centre);
	// if subquadIndex is -1 searchEnv is not contained in a subquad
	if (subnodeIndex!=-1) {
		// create the quad if it does not exist
		QuadTreeNode *node=getSubnode(subnodeIndex);
		// recursively search the found/created quad
		return node->getNode(searchEnv);
	} else {
		return this;
	}
}

/*public*/
QuadTreeNodeBase*
QuadTreeNode::find(const Envelope *searchEnv)
{
	int subnodeIndex=getSubnodeIndex(searchEnv, centre);
	if (subnodeIndex==-1)
		return this;
	if (subnode[subnodeIndex]!=NULL) {
		// query lies in subquad, so search it
		QuadTreeNode *node=subnode[subnodeIndex];
		return node->find(searchEnv);
	}
	// no existing subquad, so return this one anyway
	return this;
}

void QuadTreeNode::insertNode(QuadTreeNode* node) {
	Assert::isTrue(env==NULL || env->contains(node->env));
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
		QuadTreeNode *childNode=createSubnode(index);
		childNode->insertNode(node);
		subnode[index]=childNode;
	}
}

/**
* get the subquad for the index.
* If it doesn't exist, create it
*/
QuadTreeNode* QuadTreeNode::getSubnode(int index){
	if (subnode[index]==NULL) {
		subnode[index]=createSubnode(index);
	}
	return subnode[index];
}

QuadTreeNode* QuadTreeNode::createSubnode(int index) {
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
	QuadTreeNode *node=new QuadTreeNode(sqEnv,level-1);
	return node;
}

string
QuadTreeNode::toString() const
{
	ostringstream os;
	os <<"L"<<level<<" "<<env->toString()<<" Ctr["<<centre.toString()<<"]";
	os <<" "+QuadTreeNodeBase::toString();
	return os.str();
}

} // namespace geos.index.quadtree
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.13  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.12  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.11  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.10  2004/11/19 16:15:55  strk
 * Completely removed sprintf usage, substituted by stringstream.
 *
 * Revision 1.9  2004/11/19 16:09:53  strk
 * Added <stdio.h> include for sprintf recognition.
 *
 * Revision 1.8  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.7  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.6  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 **********************************************************************/

