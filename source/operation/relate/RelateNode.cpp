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
 * Revision 1.8  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/opRelate.h>
#include <stdio.h>

namespace geos {

RelateNode::RelateNode(Coordinate& coord,EdgeEndStar *edges): Node(coord,edges) {}
RelateNode::~RelateNode() {
//	delete edges;
}

/**
* Update the IM with the contribution for this component.
* A component only contributes if it has a labelling for both parent geometries
*/
void RelateNode::computeIM(IntersectionMatrix *im) {
	im->setAtLeastIfValid(label->getLocation(0),label->getLocation(1),0);
}

/**
* Update the IM with the contribution for the EdgeEnds incident on this node.
*/
void RelateNode::updateIMFromEdges(IntersectionMatrix *im) {
	((EdgeEndBundleStar*) edges)->updateIM(im);
}
}

