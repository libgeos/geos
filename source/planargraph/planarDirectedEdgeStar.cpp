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
 **********************************************************************
 * $Log$
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
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/planargraph.h>

namespace geos {
//namespace planargraph {

/*
 * Constructs a DirectedEdgeStar with no edges.
 */
planarDirectedEdgeStar::planarDirectedEdgeStar()
{
	outEdges = new vector<planarDirectedEdge*>();
	sorted=false;
}

planarDirectedEdgeStar::~planarDirectedEdgeStar()
{
	delete outEdges;
}

/*
 * Adds a new member to this DirectedEdgeStar.
 */
void
planarDirectedEdgeStar::add(planarDirectedEdge *de)
{
	outEdges->push_back(de);
	sorted=false;
}

/*
 * Drops a member of this DirectedEdgeStar.
 */
void
planarDirectedEdgeStar::remove(planarDirectedEdge *de)
{
	for(int i=0;i<(int)outEdges->size();i++) {
		if((*outEdges)[i]==de) {
			outEdges->erase(outEdges->begin()+i);
			i--;
		}
	}
}

/*
 * Returns an Iterator over the DirectedEdges, in ascending order
 * by angle with the positive x-axis.
 */
vector<planarDirectedEdge*>::iterator
planarDirectedEdgeStar::iterator()
{
	sortEdges();
	return outEdges->begin();
}

/*
 * Returns the number of edges around the Node associated with this
 * DirectedEdgeStar.
 */
int
planarDirectedEdgeStar::getDegree()
{ 
	return (int)outEdges->size();
}

/*
 * Returns the coordinate for the node at wich this star is based
 */
Coordinate&
planarDirectedEdgeStar::getCoordinate()
{
	if (outEdges->empty())
		return Coordinate::nullCoord;
	planarDirectedEdge *e=(*outEdges)[0];
	return e->getCoordinate();
}

/*
 * Returns the DirectedEdges, in ascending order by angle with
 * the positive x-axis.
 */
vector<planarDirectedEdge*>*
planarDirectedEdgeStar::getEdges()
{
	sortEdges();
	return outEdges;
}

bool
pdeLessThan(planarDirectedEdge *first,planarDirectedEdge * second)
{
	if (first->compareTo(second)<=0)
		return true;
	else
		return false;
}

void
planarDirectedEdgeStar::sortEdges()
{
	if (!sorted) {
		sort(outEdges->begin(),outEdges->end(),pdeLessThan);
		sorted=true;
	}
}

/*
 * Returns the zero-based index of the given Edge, after sorting in
 * ascending order by angle with the positive x-axis.
 */
int
planarDirectedEdgeStar::getIndex(planarEdge *edge)
{
	sortEdges();
	for (int i = 0; i<(int)outEdges->size(); i++) {
		planarDirectedEdge *de =(*outEdges)[i];
		if (de->getEdge() == edge)
		return i;
	}
	return -1;
}

/*
 * Returns the zero-based index of the given DirectedEdge, after sorting
 * in ascending order by angle with the positive x-axis.
 */  
int
planarDirectedEdgeStar::getIndex(planarDirectedEdge *dirEdge)
{
	sortEdges();
	for (int i = 0; i <(int) outEdges->size(); i++) {
		planarDirectedEdge *de =(*outEdges)[i];
		if (de == dirEdge)
		return i;
	}
	return -1;
}

/*
 * Returns the remainder when i is divided by the number of edges in this
 * DirectedEdgeStar. 
 */
int
planarDirectedEdgeStar::getIndex(int i)
{
	int modi = i % (int)outEdges->size();
	//I don't think modi can be 0 (assuming i is positive) [Jon Aquino 10/28/2003] 
	if (modi < 0) modi += (int)outEdges->size();
	return modi;
}

/*
 * Returns the DirectedEdge on the left-hand side of the given
 * DirectedEdge (which must be a member of this DirectedEdgeStar). 
 */
planarDirectedEdge*
planarDirectedEdgeStar::getNextEdge(planarDirectedEdge *dirEdge)
{
	int i = getIndex(dirEdge);
	return (*outEdges)[getIndex(i + 1)];
}

//} // namespace planargraph
} // namespace geos
