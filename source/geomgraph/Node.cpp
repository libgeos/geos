/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geomgraph.h>
//#include "../headers/util.h"

#ifndef DEBUG
#define DEBUG 0
#endif
#ifndef COMPUTE_Z
#define COMPUTE_Z 1
#endif

namespace geos {

Node::Node(const Coordinate& newCoord, EdgeEndStar* newEdges):
	GraphComponent(new Label(0,Location::UNDEF)),
	coord(newCoord),
	edges(newEdges)

{
#if DEBUG
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
}

Node::~Node()
{
#if DEBUG
	cerr<<"["<<this<<"] Node::~Node()"<<endl;
#endif
	delete edges;
}

const Coordinate&
Node::getCoordinate() const
{
	return coord;
}

EdgeEndStar *
Node::getEdges()
{
	return edges;
}

bool
Node::isIsolated() const
{
	return (label->getGeometryCount()==1);
}

bool
Node::isIncidentEdgeInResult() const
{
	EdgeEndStar::iterator it=edges->begin();
	EdgeEndStar::iterator endIt=edges->end();
	for ( ; it!=endIt; ++it)
	{
		DirectedEdge *de = (DirectedEdge *)(*it);
		if ( de->getEdge()->isInResult() ) return true;
	}
	return false;
}

void
Node::add(EdgeEnd *e)
{
#if DEBUG
	cerr<<"["<<this<<"] Node::add("<<e->print()<<")"<<endl;
#endif
	// Assert: start pt of e is equal to node point
	if (edges==NULL) return;

	edges->insert(e);
	e->setNode(this);
#if COMPUTE_Z
	addZ(e->getCoordinate().z);
#endif
}

void
Node::mergeLabel(const Node* n)
{
	mergeLabel(n->label);
}

void
Node::mergeLabel(const Label* label2)
{
	for (int i=0; i<2; i++) {
		int loc=computeMergedLocation(label2, i);
		int thisLoc=label->getLocation(i);
		if (thisLoc==Location::UNDEF) label->setLocation(i,loc);
	}
}

void
Node::setLabel(int argIndex, int onLocation)
{
	if (label==NULL) {
		label=new Label(argIndex, onLocation);
	} else
		label->setLocation(argIndex, onLocation);
}

void
Node::setLabelBoundary(int argIndex)
{
	int loc=Location::UNDEF;
	if (label!=NULL)
		loc=label->getLocation(argIndex);
	// flip the loc
	int newLoc;
	switch (loc){
		case Location::BOUNDARY: newLoc=Location::INTERIOR; break;
		case Location::INTERIOR: newLoc=Location::BOUNDARY; break;
		default: newLoc=Location::BOUNDARY;  break;
	}
	label->setLocation(argIndex, newLoc);
}

int
Node::computeMergedLocation(const Label* label2, int eltIndex)
{
	int loc=Location::UNDEF;
	loc=label->getLocation(eltIndex);
	if (!label2->isNull(eltIndex)) {
		int nLoc=label2->getLocation(eltIndex);
		if (loc!=Location::BOUNDARY) loc=nLoc;
	}
	return loc;
}

string
Node::print()
{
	string out="node "+coord.toString()+" lbl: "+label->toString();
	return out;
}

void
Node::addZ(double z)
{
#if DEBUG
	cerr<<"["<<this<<"] Node::addZ("<<z<<")";
#endif
	if ( ISNAN(z) )
	{
#if DEBUG
		cerr<<" skipped"<<endl;
#endif
		return;
	}
	for (unsigned int i=0; i<zvals.size(); i++) if ( zvals[i] == z )
	{
#if DEBUG
		cerr<<" already stored"<<endl;
#endif
		return;
	}
	zvals.push_back(z);
	ztot+=z;
	coord.z=ztot/zvals.size();
#if DEBUG
	cerr<<" added "<<z<<": ["<<ztot<<"/"<<zvals.size()<<"="<<coord.z<<"]"<<endl;
#endif
}

const vector<double>&
Node::getZ() const
{
	return zvals;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2005/11/29 00:48:35  strk
 * Removed edgeList cache from EdgeEndRing. edgeMap is enough.
 * Restructured iterated access by use of standard ::iterator abstraction
 * with scoped typedefs.
 *
 * Revision 1.15  2005/11/21 16:03:20  strk
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
 * Revision 1.14  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.13  2005/11/09 13:44:28  strk
 * Cleanups in Node and NodeMap.
 * Optimization of EdgeIntersectionLessThen.
 *
 * Revision 1.12  2005/06/28 00:04:48  strk
 * improved ::isIncidentEdgeInResult() method
 *
 * Revision 1.11  2005/06/25 10:20:39  strk
 * OverlayOp speedup (JTS port)
 *
 * Revision 1.10  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.9  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.8  2004/11/26 09:53:48  strk
 * Added more FINITE calls, and added inf and -inf to FINITE checks
 *
 * Revision 1.7  2004/11/20 15:41:18  strk
 * Added management of vector of composing Z values.
 *
 * Revision 1.6  2004/11/19 10:10:23  strk
 * COMPUTE_Z re-enabled by default
 *
 * Revision 1.5  2004/11/17 15:09:08  strk
 * Changed COMPUTE_Z defaults to be more conservative
 *
 * Revision 1.4  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
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

