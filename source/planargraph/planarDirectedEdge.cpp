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

#include <geos/planargraph.h>
#include <geos/geomgraph.h>
#include <cmath>
#include <typeinfo>
#include <sstream>

namespace geos {
namespace planargraph {

/*
 * Returns a List containing the parent planarEdge (possibly null)
 * for each of the given planarDirectedEdges.
 */
vector<planarEdge*>*
planarDirectedEdge::toEdges(vector<planarDirectedEdge*> *dirEdges) 
{
	vector<planarEdge*> *edges=new vector<planarEdge*>();
	for (int i=0;i<(int)dirEdges->size();i++) {
		edges->push_back((*dirEdges)[i]->parentEdge);
	}
	return edges;
}

//const CGAlgorithms* planarDirectedEdge::cga=new CGAlgorithms();

/**
* Constructs a planarDirectedEdge connecting the <code>from</code> node to the
* <code>to</code> node.
*
* @param directionPt
*                  specifies this planarDirectedEdge's direction (given by an imaginary
*                  line from the <code>from</code> node to <code>directionPt</code>)
* @param edgeDirection
*                  whether this planarDirectedEdge's direction is the same as or
*                  opposite to that of the parent planarEdge (if any)
*/
planarDirectedEdge::planarDirectedEdge(planarNode* newFrom, planarNode* newTo,
	const Coordinate &directionPt, bool newEdgeDirection)
{
	from=newFrom;
	to=newTo;
	edgeDirection=newEdgeDirection;
	p0=from->getCoordinate();
	p1=directionPt;
	double dx = p1.x - p0.x;
	double dy = p1.y - p0.y;
	quadrant = geomgraph::Quadrant::quadrant(dx, dy);
	angle=atan2(dy, dx);
	//Assert.isTrue(! (dx == 0 && dy == 0), "EdgeEnd with identical endpoints found");
}

/*
 * Returns this planarDirectedEdge's parent planarEdge,
 * or null if it has none.
 */
planarEdge*
planarDirectedEdge::getEdge() const
{
	return parentEdge;
}
/*
 * Associates this planarDirectedEdge with an planarEdge
 * (possibly null, indicating no associated planarEdge).
 */
void
planarDirectedEdge::setEdge(planarEdge* newParentEdge)
{ 
	parentEdge=newParentEdge;
}

/**
* Returns 0, 1, 2, or 3, indicating the quadrant in which this planarDirectedEdge's
* orientation lies.
*/
int
planarDirectedEdge::getQuadrant() const
{
	return quadrant;
}
/**
* Returns a point to which an imaginary line is drawn from the from-node to
* specify this planarDirectedEdge's orientation.
*/
const Coordinate&
planarDirectedEdge::getDirectionPt() const
{
	return p1;
}
/**
* Returns whether the direction of the parent planarEdge (if any) is the same as that
* of this Directed planarEdge.
*/
bool planarDirectedEdge::getEdgeDirection() const { 
	return edgeDirection;
}
/**
* Returns the node from which this planarDirectedEdge leaves.
*/
planarNode*
planarDirectedEdge::getFromNode() const
{
	return from;
}
/**
* Returns the node to which this planarDirectedEdge goes.
*/
planarNode*
planarDirectedEdge::getToNode() const
{ 
	return to;
}
/**
* Returns the coordinate of the from-node.
*/
Coordinate&
planarDirectedEdge::getCoordinate() const
{ 
	return from->getCoordinate();
}
/**
* Returns the angle that the start of this planarDirectedEdge makes with the
* positive x-axis, in radians.
*/
double
planarDirectedEdge::getAngle() const
{ 
	return angle;
}
/**
* Returns the symmetric planarDirectedEdge -- the other planarDirectedEdge associated with
* this planarDirectedEdge's parent planarEdge.
*/
planarDirectedEdge*
planarDirectedEdge::getSym() const
{ 
	return sym;
}

/*
 * Sets this planarDirectedEdge's symmetric planarDirectedEdge,
 * which runs in the opposite direction.
 */
void
planarDirectedEdge::setSym(planarDirectedEdge *newSym)
{ 
	sym = newSym;
}

/**
 * Returns 1 if this planarDirectedEdge has a greater angle with the
 * positive x-axis than b", 0 if the planarDirectedEdges are collinear, and -1 otherwise.
 * <p>
 * Using the obvious algorithm of simply computing the angle is not robust,
 * since the angle calculation is susceptible to roundoff. A robust algorithm
 * is:
 * <ul>
 * <li>first compare the quadrants. If the quadrants are different, it it
 * trivial to determine which vector is "greater".
 * <li>if the vectors lie in the same quadrant, the robust
 * {@link RobustCGAlgorithms#computeOrientation(Coordinate, Coordinate, Coordinate)}
 * function can be used to decide the relative orientation of the vectors.
 * </ul>
 */
int
planarDirectedEdge::compareTo(const planarDirectedEdge* de) const
{
	return compareDirection(de);
}

/**
* Returns 1 if this planarDirectedEdge has a greater angle with the
* positive x-axis than b", 0 if the planarDirectedEdges are collinear, and -1 otherwise.
* <p>
* Using the obvious algorithm of simply computing the angle is not robust,
* since the angle calculation is susceptible to roundoff. A robust algorithm
* is:
* <ul>
* <li>first compare the quadrants. If the quadrants are different, it it
* trivial to determine which vector is "greater".
* <li>if the vectors lie in the same quadrant, the robust
* {@link RobustCGAlgorithms#computeOrientation(Coordinate, Coordinate, Coordinate)}
* function can be used to decide the relative orientation of the vectors.
* </ul>
*/
int
planarDirectedEdge::compareDirection(const planarDirectedEdge *e) const
{
// if the rays are in different quadrants, determining the ordering is trivial
	if (quadrant > e->quadrant) return 1;
	if (quadrant < e->quadrant) return -1;
	// vectors are in the same quadrant - check relative orientation of direction vectors
	// this is > e if it is CCW of e
	return cga.computeOrientation(e->p0,e->p1,p1);
}

/**
* Prints a detailed string representation of this planarDirectedEdge to the given PrintStream.
*/
string
planarDirectedEdge::print() const
{
	ostringstream s;
	s<<typeid(*this).name()<<": "<<p0.toString()<<" - "<<p1.toString();
	s<<" "<<quadrant<<":"<<angle;
	return s.str();
}


} // namespace planargraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.9  2005/04/06 11:09:41  strk
 * Applied patch from Jon Schlueter (math.h => cmath; ieeefp.h in "C" block)
 *
 * Revision 1.8  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.7  2004/12/14 10:35:44  strk
 * Comments cleanup. PolygonizeGraph keeps track of generated CoordinateSequence
 * for delayed destruction.
 *
 * Revision 1.6  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.5  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.4  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.3  2004/06/15 07:40:30  strk
 * Added missing <stdio.h> include
 *
 * Revision 1.2  2004/05/03 10:43:43  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

