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
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/indexStrtree.h"
#include "../../headers/util.h"
#include "stdio.h"

namespace geos {

AbstractNode::AbstractNode(int newLevel) {
	childBoundables=new vector<Boundable*>();
	bounds=NULL;
	level=newLevel;
}

AbstractNode::~AbstractNode() {
	delete childBoundables;
}

vector<Boundable*>* AbstractNode::getChildBoundables() {
	return childBoundables;
}

void* AbstractNode::getBounds() {
	if (bounds==NULL) {
		bounds = computeBounds();
	}
	return bounds;
}

int AbstractNode::getLevel() {
	return level;
}

/**
*@param  childBoundable  either a Node or an ItemBoundable
*/
void AbstractNode::addChildBoundable(Boundable *childBoundable) {
	Assert::isTrue(bounds==NULL);
	childBoundables->push_back(childBoundable);
}
}

