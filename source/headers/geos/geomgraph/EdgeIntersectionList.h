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
 * Last port: geomgraph/EdgeIntersectionList.java rev. 1.5 (JTS-1.10)
 *
 * NON-EXPOSED GEOS HEADER
 *
 **********************************************************************/


#ifndef GEOS_GEOMGRAPH_EDGEINTERSECTIONLIST_H
#define GEOS_GEOMGRAPH_EDGEINTERSECTIONLIST_H

#include <vector>
#include <set>
#include <string>

#include <geos/geomgraph/EdgeIntersection.h> // for EdgeIntersectionLessThen
#include <geos/geom/Coordinate.h> // for CoordinateLessThen

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
	namespace geomgraph {
		class Edge;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph


/**
 * A list of edge intersections along an Edge.
 * Implements splitting an edge with intersections
 * into multiple resultant edges.
 */
class EdgeIntersectionList{
public:
	typedef std::set<EdgeIntersection *, EdgeIntersectionLessThen> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;

private:
	container nodeMap;

public:

	Edge *edge;
	EdgeIntersectionList(Edge *edge);
	~EdgeIntersectionList();

	/*
	 * Adds an intersection into the list, if it isn't already there.
	 * The input segmentIndex and dist are expected to be normalized.
	 * @return the EdgeIntersection found or added
	 */
	EdgeIntersection* add(const geom::Coordinate& coord,
		int segmentIndex, double dist);

	iterator begin() { return nodeMap.begin(); }
	iterator end() { return nodeMap.end(); }
	const_iterator begin() const { return nodeMap.begin(); }
	const_iterator end() const { return nodeMap.end(); }

	bool isEmpty() const;
	bool isIntersection(const geom::Coordinate& pt) const;

	/*
	 * Adds entries for the first and last points of the edge to the list
	 */
	void addEndpoints();

	/**
	 * Creates new edges for all the edges that the intersections in this
	 * list split the parent edge into.
	 * Adds the edges to the input list (this is so a single list
	 * can be used to accumulate all split edges for a Geometry).
	 *
	 * @param edgeList a list of EdgeIntersections
	 */
	void addSplitEdges(std::vector<Edge*> *edgeList);

	Edge *createSplitEdge(EdgeIntersection *ei0, EdgeIntersection *ei1);
	std::string print() const;

};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geomgraph/EdgeIntersectionList.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGEINTERSECTIONLIST_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

