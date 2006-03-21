/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/planargraph/Node.h>

#include <vector>
#include <iostream>

using namespace std;

namespace geos {
namespace planargraph {

/**
* Returns all Edges that connect the two nodes (which are assumed to be different).
*/
vector<Edge*>*
Node::getEdgesBetween(Node *node0, Node *node1)
{
	return NULL;
}

std::ostream& operator<<(std::ostream& os, const Node& n) {
	os << "Node " << n.pt << " with degree " << n.getDegree();
	if ( n.isMarked() ) os << " Marked ";
	if ( n.isVisited() ) os << " Visited ";
	return os;
}

} // namespace planargraph 
} // namespace geos 

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 **********************************************************************/
