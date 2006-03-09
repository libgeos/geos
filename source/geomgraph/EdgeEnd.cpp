/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <typeinfo>
#include <cmath>
#include <sstream>
#include <string>
#include <cassert>

//#include <geos/geomgraph.h>
//#include <geos/util.h>
#include <geos/geomgraph/EdgeEnd.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/Quadrant.h>
#include <geos/geom/Coordinate.h>

//using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

using namespace geos::algorithm;

//CGAlgorithms* EdgeEnd::cga=new RobustCGAlgorithms();

EdgeEnd::EdgeEnd()
	:
	edge(NULL),
	label(NULL),
	node(NULL),
	dx(0.0),
	dy(0.0),
	quadrant(0)
{
}

EdgeEnd::~EdgeEnd()
{
	delete label;
}

EdgeEnd::EdgeEnd(Edge* newEdge)
	:
	edge(newEdge),
	label(NULL),
	node(NULL),
	dx(0.0),
	dy(0.0),
	quadrant(0)
{
}

EdgeEnd::EdgeEnd(Edge* newEdge,
		const Coordinate& newP0, const Coordinate& newP1):
	edge(newEdge),
	label(NULL),
	node(NULL),
	dx(0.0),
	dy(0.0),
	quadrant(0)
{
	init(newP0,newP1);
}

EdgeEnd::EdgeEnd(Edge* newEdge, const Coordinate& newP0,
		const Coordinate& newP1, Label* newLabel):
	edge(newEdge),
	label(newLabel),
	node(NULL),
	dx(0.0),
	dy(0.0),
	quadrant(0)
{
	init(newP0,newP1);
}

void
EdgeEnd::init(const Coordinate& newP0, const Coordinate& newP1)
{
	p0=newP0;
	p1=newP1;
	dx=p1.x-p0.x;
	dy=p1.y-p0.y;
	quadrant=Quadrant::quadrant(dx,dy);

	// "EdgeEnd with identical endpoints found");
	assert(!(dx == 0 && dy == 0));
}

Edge* EdgeEnd::getEdge() {return edge;}
Label* EdgeEnd::getLabel() {return label;}
Coordinate& EdgeEnd::getCoordinate() {return p0;}
Coordinate& EdgeEnd::getDirectedCoordinate() {return p1;}
int EdgeEnd::getQuadrant() {return quadrant;}
double EdgeEnd::getDx() {return dx;}
double EdgeEnd::getDy() {return dy;}
void EdgeEnd::setNode(Node* newNode) {node=newNode;}
Node* EdgeEnd::getNode() {return node;}

int
EdgeEnd::compareTo(const EdgeEnd* e) const
{
	return compareDirection(e);
}

int
EdgeEnd::compareDirection(const EdgeEnd* e) const
{
	if (dx == e->dx && dy == e->dy)
		return 0;
	// if the rays are in different quadrants, determining the ordering is trivial
	if (quadrant>e->quadrant) return 1;
	if (quadrant<e->quadrant) return -1;
	// vectors are in the same quadrant - check relative orientation of direction vectors
	// this is > e if it is CCW of e
	return CGAlgorithms::computeOrientation(e->p0,e->p1,p1);
}

void
EdgeEnd::computeLabel()
{
	// subclasses should override this if they are using labels
}

std::string
EdgeEnd::print()
{
	std::ostringstream s;

	s<<"EdgeEnd: ";
	s<<p0.toString();
	s<<" - ";
	s<<p1.toString();
	s<<" ";
	s<<quadrant<<":"<<atan2(dy,dx);
	s<<"  ";
	s<<label->toString();

	return s.str();
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.13  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.12  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.11  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.10  2006/02/28 14:34:04  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.9  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.8  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.7  2005/11/16 22:21:45  strk
 * enforced const-correctness and use of initializer lists.
 *
 * Revision 1.6  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.5  2005/04/06 11:09:41  strk
 * Applied patch from Jon Schlueter (math.h => cmath; ieeefp.h in "C" block)
 *
 * Revision 1.4  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.3  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/
