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
 * Revision 1.7  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.6  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2004/03/25 02:23:55  ybychkov
 * All "index/*" packages upgraded to JTS 1.4
 *
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/indexStrtree.h>
#include <geos/util.h>
#include <stdio.h>

namespace geos {

/**
* Constructs an AbstractNode at the given level in the tree
* @param level 0 if this node is a leaf, 1 if a parent of a leaf, and so on; the
* root node will have the highest level
*/
AbstractNode::AbstractNode(int newLevel) {
	childBoundables=new vector<Boundable*>();
	bounds=NULL;
	level=newLevel;
}

AbstractNode::~AbstractNode() {
	delete childBoundables;
}

/**
* Returns either child AbstractNodes, or if this is a leaf node, real data (wrapped
* in ItemBoundables).
*/
vector<Boundable*>* AbstractNode::getChildBoundables() {
	return childBoundables;
}

const void *
AbstractNode::getBounds()
{
	if (bounds==NULL) {
		bounds = computeBounds();
	}
	return bounds;
}

/**
* Returns 0 if this node is a leaf, 1 if a parent of a leaf, and so on; the
* root node will have the highest level
*/
int AbstractNode::getLevel() {
	return level;
}

/**
* Adds either an AbstractNode, or if this is a leaf node, a data object
* (wrapped in an ItemBoundable)
*/
void AbstractNode::addChildBoundable(Boundable *childBoundable) {
	Assert::isTrue(bounds==NULL);
	childBoundables->push_back(childBoundable);
}
}

