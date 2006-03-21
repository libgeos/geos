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
 **********************************************************************/

#include <geos/opPolygonize.h>
#include <geos/planargraph/DirectedEdge.h>

using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

/**
 * Constructs a directed edge connecting the <code>from</code> node to the
 * <code>to</code> node.
 *
 * @param directionPt
 *        specifies this DirectedEdge's direction (given by an imaginary
 *        line from the <code>from</code> node to <code>directionPt</code>)
 *
 * @param edgeDirection
 *        whether this DirectedEdge's direction is the same as or
 *        opposite to that of the parent Edge (if any)
 */
PolygonizeDirectedEdge::PolygonizeDirectedEdge(Node *newFrom,
		Node *newTo, const Coordinate& newDirectionPt,
		bool nEdgeDirection)
	:
	DirectedEdge(newFrom, newTo,
		newDirectionPt, nEdgeDirection)
{
	edgeRing=NULL;
	next=NULL;
	label=-1;
}

/*
 * Returns the identifier attached to this directed edge.
 */
long
PolygonizeDirectedEdge::getLabel() const
{ 
	return label;
}

/*
 * Attaches an identifier to this directed edge.
 */
void PolygonizeDirectedEdge::setLabel(long newLabel) { 
	label=newLabel;
}

/*
 * Returns the next directed edge in the EdgeRing that this directed
 * edge is a member of.
 */
PolygonizeDirectedEdge *
PolygonizeDirectedEdge::getNext() const
{
	return next;
}

/*
 * Sets the next directed edge in the EdgeRing that this directed
 * edge is a member of.
 */
void
PolygonizeDirectedEdge::setNext(PolygonizeDirectedEdge *newNext)
{
	next=newNext;
}

/*
 * Returns the ring of directed edges that this directed edge is
 * a member of, or null if the ring has not been set.
 * @see #setRing(EdgeRing)
 */
bool
PolygonizeDirectedEdge::isInRing() const
{ 
	return edgeRing!=NULL;
}

/*
 * Sets the ring of directed edges that this directed edge is
 * a member of.
 */
void
PolygonizeDirectedEdge::setRing(polygonizeEdgeRing *newEdgeRing)
{
	edgeRing=newEdgeRing;
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 * Revision 1.5  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.4  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.3  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/

