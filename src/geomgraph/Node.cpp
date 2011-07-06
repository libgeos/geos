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
 **********************************************************************
 *
 * Last port: geomgraph/Node.java r411 (JTS-1.12+)
 *
 **********************************************************************/

#include <geos/platform.h>
#include <geos/geom/Coordinate.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/EdgeEndStar.h>
#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geom/Location.h>

#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif
#ifndef COMPUTE_Z
#define COMPUTE_Z 1
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

/*public*/
Node::Node(const Coordinate& newCoord, EdgeEndStar* newEdges)
	:
	GraphComponent(new Label(0,Location::UNDEF)),
	coord(newCoord),
	edges(newEdges)

{
#if GEOS_DEBUG
	cerr<<"["<<this<<"] Node::Node("<<newCoord.toString()<<")"<<endl;
#endif

#if COMPUTE_Z
	ztot = 0;
	addZ(newCoord.z);
	if ( edges )
	{
		EdgeEndStar::iterator endIt = edges->end();
		for (EdgeEndStar::iterator it=edges->begin(); it!=endIt; ++it)
		{
			EdgeEnd *ee = *it;
			addZ(ee->getCoordinate().z);
		}
	}
#endif // COMPUTE_Z

	testInvariant();
}

/*public*/
Node::~Node()
{
	testInvariant();
#if GEOS_DEBUG
	cerr<<"["<<this<<"] Node::~Node()"<<endl;
#endif
	delete edges;
}

/*public*/
const Coordinate&
Node::getCoordinate() const
{
	testInvariant();
	return coord;
}

/*public*/
EdgeEndStar *
Node::getEdges()
{
	testInvariant();

	return edges;
}

/*public*/
bool
Node::isIsolated() const
{
	testInvariant();

	return (label->getGeometryCount()==1);
}

/*public*/
bool
Node::isIncidentEdgeInResult() const
{
	testInvariant();

	if (!edges) return false;

	EdgeEndStar::iterator it=edges->begin();
	EdgeEndStar::iterator endIt=edges->end();
	for ( ; it!=endIt; ++it)
	{
		assert(*it);
		assert(dynamic_cast<DirectedEdge *>(*it));
		DirectedEdge *de = static_cast<DirectedEdge *>(*it);
		if ( de->getEdge()->isInResult() ) return true;
	}
	return false;
}

void
Node::add(EdgeEnd *e)
{
	assert(e);
#if GEOS_DEBUG
	cerr<<"["<<this<<"] Node::add("<<e->print()<<")"<<endl;
#endif
	// Assert: start pt of e is equal to node point
	assert(e->getCoordinate().equals2D(coord));

	// It seems it's legal for edges to be NULL
	// we'd not be honouring the promise of adding
	// an EdgeEnd in this case, though ...
	assert(edges);
	//if (edges==NULL) return;

	edges->insert(e);
	e->setNode(this);
#if COMPUTE_Z
	addZ(e->getCoordinate().z);
#endif
	testInvariant();
}

/*public*/
void
Node::mergeLabel(const Node& n)
{
	assert(n.label);
	mergeLabel(*(n.label));
	testInvariant();
}

/*public*/
void
Node::mergeLabel(const Label& label2)
{
	for (int i=0; i<2; i++) {
		int loc=computeMergedLocation(&label2, i);
		int thisLoc=label->getLocation(i);
		if (thisLoc==Location::UNDEF) label->setLocation(i,loc);
	}
	testInvariant();
}

/*public*/
void
Node::setLabel(int argIndex, int onLocation)
{
	if (label==NULL) {
		label=new Label(argIndex, onLocation);
	} else
		label->setLocation(argIndex, onLocation);

	testInvariant();
}

/*public*/
void
Node::setLabelBoundary(int argIndex)
{
	// See https://sourceforge.net/tracker/?func=detail&aid=3353871&group_id=128875&atid=713120
	if (label == NULL) return;

	int loc = label->getLocation(argIndex);
	// flip the loc
	int newLoc;
	switch (loc){
		case Location::BOUNDARY: newLoc=Location::INTERIOR; break;
		case Location::INTERIOR: newLoc=Location::BOUNDARY; break;
		default: newLoc=Location::BOUNDARY;  break;
	}
	label->setLocation(argIndex, newLoc);

	testInvariant();
}

/*public*/
int
Node::computeMergedLocation(const Label* label2, int eltIndex)
{
	int loc=Location::UNDEF;
	loc=label->getLocation(eltIndex);
	if (!label2->isNull(eltIndex)) {
		int nLoc=label2->getLocation(eltIndex);
		if (loc!=Location::BOUNDARY) loc=nLoc;
	}

	testInvariant();

	return loc;
}

/*public*/
string
Node::print()
{
	testInvariant();

	ostringstream ss;
	ss<<*this;
	return ss.str();
}

/*public*/
void
Node::addZ(double z)
{
#if GEOS_DEBUG
	cerr<<"["<<this<<"] Node::addZ("<<z<<")";
#endif
	if ( ISNAN(z) )
	{
#if GEOS_DEBUG
		cerr<<" skipped"<<endl;
#endif
		return;
	}
	if ( find(zvals.begin(), zvals.end(), z) != zvals.end() )
	{
#if GEOS_DEBUG
		cerr<<" already stored"<<endl;
#endif
		return;
	}
	zvals.push_back(z);
	ztot+=z;
	coord.z=ztot/zvals.size();
#if GEOS_DEBUG
	cerr<<" added "<<z<<": ["<<ztot<<"/"<<zvals.size()<<"="<<coord.z<<"]"<<endl;
#endif
}

/*public*/
const vector<double>&
Node::getZ() const
{
	return zvals;
}

std::ostream& operator<< (std::ostream& os, const Node& node)
{
	os << "Node["<<&node<<"]" << std::endl
	   << "  POINT(" << node.coord << ")" << std::endl
	   << "  lbl: "+node.label->toString();
	return os;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.24  2006/04/27 15:03:48  strk
 * standard algorithm used in addZ() for vector seek
 *
 * Revision 1.23  2006/04/07 16:01:51  strk
 * Port info, doxygen comments, testInvariant(), many assertionss, handling of
 * the NULL EdgeEndStar member
 *
 * Revision 1.22  2006/03/15 16:27:54  strk
 * operator<< for Node class
 *
 * Revision 1.21  2006/03/14 15:31:39  strk
 * Cleaned up toString funx (more WKT friendly)
 *
 * Revision 1.20  2006/03/14 12:55:55  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.19  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.18  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.17  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.16  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 **********************************************************************/

