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
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/opLinemerge.h"
#include "../../headers/util.h"

namespace geos {

/**
* Constructs a LineMergeDirectedEdge connecting the <code>from</code> node to the
* <code>to</code> node.
*
* @param directionPt
*                  specifies this DirectedEdge's direction (given by an imaginary
*                  line from the <code>from</code> node to <code>directionPt</code>)
* @param edgeDirection
*                  whether this DirectedEdge's direction is the same as or
*                  opposite to that of the parent Edge (if any)
*/  
LineMergeDirectedEdge::LineMergeDirectedEdge(planarNode *newFrom,planarNode *newTo,Coordinate& newDirectionPt,bool nEdgeDirection):
	planarDirectedEdge(newFrom,newTo,newDirectionPt,nEdgeDirection) {}

/**
* Returns the directed edge that starts at this directed edge's end point, or null
* if there are zero or multiple directed edges starting there.  
* @return
*/
LineMergeDirectedEdge* LineMergeDirectedEdge::getNext() {
	if (getToNode()->getDegree()!=2) {
		return NULL;
	}
	if ((*(getToNode()->getOutEdges()->getEdges()))[0]==getSym()) {
		return (LineMergeDirectedEdge*) (*(getToNode()->getOutEdges()->getEdges()))[1];
	}
	Assert::isTrue((*(getToNode()->getOutEdges()->getEdges()))[1]==getSym());
	return (LineMergeDirectedEdge*) (*(getToNode()->getOutEdges()->getEdges()))[0];
}
}
