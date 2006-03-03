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
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/planargraph.h>
#include <vector>

using namespace std;

namespace geos {
namespace planargraph {


/*
 * Adds a new member to this DirectedEdgeStar.
 */
void
planarDirectedEdgeStar::add(planarDirectedEdge *de)
{
	outEdges.push_back(de);
	sorted=false;
}

/*
 * Drops a member of this DirectedEdgeStar.
 */
void
planarDirectedEdgeStar::remove(planarDirectedEdge *de)
{
	for(unsigned int i=0; i<outEdges.size(); ++i)
	{
		if(outEdges[i]==de)
		{
			outEdges.erase(outEdges.begin()+i);
			--i;
		}
	}
}

vector<planarDirectedEdge*>::iterator
planarDirectedEdgeStar::begin() {
	sortEdges();
	return outEdges.begin();
}

vector<planarDirectedEdge*>::iterator
planarDirectedEdgeStar::end() {
	sortEdges();
	return outEdges.end();
}

vector<planarDirectedEdge*>::const_iterator
planarDirectedEdgeStar::begin() const {
	sortEdges();
	return outEdges.begin();
}

vector<planarDirectedEdge*>::const_iterator
planarDirectedEdgeStar::end() const {
	sortEdges();
	return outEdges.end();
}

/*
 * Returns the coordinate for the node at wich this star is based
 */
Coordinate&
planarDirectedEdgeStar::getCoordinate() const
{
	if (outEdges.empty())
		return Coordinate::getNull();
	planarDirectedEdge *e=outEdges[0];
	return e->getCoordinate();
}

/*
 * Returns the DirectedEdges, in ascending order by angle with
 * the positive x-axis.
 */
vector<planarDirectedEdge*>&
planarDirectedEdgeStar::getEdges()
{
	sortEdges();
	return outEdges;
}

bool
pdeLessThan(planarDirectedEdge *first, planarDirectedEdge * second)
{
	if (first->compareTo(second)<0)
		return true;
	else
		return false;
}

/*private*/
void
planarDirectedEdgeStar::sortEdges() const
{
	if (!sorted) {
		sort(outEdges.begin(), outEdges.end(), pdeLessThan);
		sorted=true;
	}
}

/*
 * Returns the zero-based index of the given Edge, after sorting in
 * ascending order by angle with the positive x-axis.
 */
int
planarDirectedEdgeStar::getIndex(const planarEdge *edge)
{
	sortEdges();
	for (unsigned int i = 0; i<outEdges.size(); ++i)
	{
		planarDirectedEdge *de =outEdges[i];
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
planarDirectedEdgeStar::getIndex(const planarDirectedEdge *dirEdge)
{
	sortEdges();
	for (unsigned int i = 0; i <outEdges.size(); ++i)
	{
		planarDirectedEdge *de =outEdges[i];
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
planarDirectedEdgeStar::getIndex(int i) const
{
	int modi = i % (int)outEdges.size();
	//I don't think modi can be 0 (assuming i is positive) [Jon Aquino 10/28/2003] 
	if (modi < 0) modi += (int)outEdges.size();
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
	return outEdges[getIndex(i + 1)];
}

} // namespace planargraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.10  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.9  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.8  2006/02/08 12:59:56  strk
 * - NEW Geometry::applyComponentFilter() templated method
 * - Changed Geometry::getGeometryN() to take unsigned int and getNumGeometries
 *   to return unsigned int.
 * - Changed planarNode::getDegree() to return unsigned int.
 * - Added Geometry::NonConstVect typedef
 * - NEW LineSequencer class
 * - Changed planarDirectedEdgeStar::outEdges from protected to private
 * - added static templated setVisitedMap to change Visited flag
 *   for all values in a map
 * - Added const versions of some planarDirectedEdgeStar methods.
 * - Added containers typedefs for planarDirectedEdgeStar
 *
 * Revision 1.7  2006/02/05 17:14:43  strk
 * - New ConnectedSubgraphFinder class.
 * - More iterators returning methods, inlining and cleanups
 *   in planargraph.
 *
 * Revision 1.6  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.5  2005/08/22 13:31:17  strk
 * Fixed comparator functions used with STL sort() algorithm to
 * implement StrictWeakOrdering semantic.
 *
 * Revision 1.4  2004/12/14 10:35:44  strk
 * Comments cleanup. PolygonizeGraph keeps track of generated CoordinateSequence
 * for delayed destruction.
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
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/

