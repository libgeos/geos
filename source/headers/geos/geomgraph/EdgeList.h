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


#ifndef GEOS_GEOMGRAPH_EDGELIST_H
#define GEOS_GEOMGRAPH_EDGELIST_H

#include <vector>
#include <string>
#include <iostream>

#include <geos/indexQuadtree.h> // for inlined ctor

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace index {
		class SpatialIndex;
	}
	namespace geomgraph {
		class Edge;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class EdgeList {

private:

	std::vector<Edge*> edges;

	/**
	 * An index of the edges, for fast lookup.
	 *
	 * a Quadtree is used, because this index needs to be dynamic
	 * (e.g. allow insertions after queries).
	 * An alternative would be to use an ordered set based on the values
	 * of the edge coordinates
	 *
	 */
	geos::index::SpatialIndex* index;

public:
	friend std::ostream& operator<< (std::ostream& os, const EdgeList& el);

	EdgeList()
		:
		edges(),
		index(new geos::index::quadtree::Quadtree())
	{}

	virtual ~EdgeList() { delete index; }

	/**
	 * Insert an edge unless it is already in the list
	 */
	void add(Edge *e);

	void addAll(const std::vector<Edge*> &edgeColl);

	std::vector<Edge*> &getEdges() { return edges; }

	Edge* findEqualEdge(Edge* e);

	Edge* get(int i);

	int findEdgeIndex(Edge *e);

	std::string print();

};

std::ostream& operator<< (std::ostream& os, const EdgeList& el);


} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/EdgeList.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGELIST_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/14 11:03:15  strk
 * Added operator<< for Edge and EdgeList
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

