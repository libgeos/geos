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
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/opPolygonize.h"

namespace geos {

/**
* Constructs a directed edge connecting the <code>from</code> node to the
* <code>to</code> node.
*
* @param directionPt
*                  specifies this DirectedEdge's direction (given by an imaginary
*                  line from the <code>from</code> node to <code>directionPt</code>)
* @param edgeDirection
*                  whether this DirectedEdge's direction is the same as or
*                  opposite to that of the parent Edge (if any)
*/
PolygonizeDirectedEdge::PolygonizeDirectedEdge(planarNode *newFrom,planarNode *newTo,Coordinate& newDirectionPt,bool nEdgeDirection):
	planarDirectedEdge(newFrom,newTo,newDirectionPt,nEdgeDirection) {
		edgeRing=NULL;
		next=NULL;
		label=-1;
}

/**
* Returns the identifier attached to this directed edge.
*/
long PolygonizeDirectedEdge::getLabel() { 
	return label;
}
/**
* Attaches an identifier to this directed edge.
*/
void PolygonizeDirectedEdge::setLabel(long newLabel) { 
	label=newLabel;
}
/**
* Returns the next directed edge in the EdgeRing that this directed edge is a member
* of.
*/
PolygonizeDirectedEdge* PolygonizeDirectedEdge::getNext()  {
	return next;
}
/**
* Sets the next directed edge in the EdgeRing that this directed edge is a member
* of.
*/
void PolygonizeDirectedEdge::setNext(PolygonizeDirectedEdge *newNext)  {
	next=newNext;
}
/**
* Returns the ring of directed edges that this directed edge is
* a member of, or null if the ring has not been set.
* @see #setRing(EdgeRing)
*/
bool PolygonizeDirectedEdge::isInRing() { 
	return edgeRing!=NULL;
}
/**
* Sets the ring of directed edges that this directed edge is
* a member of.
*/
void PolygonizeDirectedEdge::setRing(polygonizeEdgeRing *newEdgeRing) {
	edgeRing=newEdgeRing;
}

}
