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


#ifndef GEOS_GEOMGRAPH_NODEMAP_H
#define GEOS_GEOMGRAPH_NODEMAP_H

#include <map>
#include <vector>
#include <string>

#include <geos/geom/Coordinate.h> // for CoordinateLessThen

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geomgraph {
		class Node;
		class EdgeEnd;
		class NodeFactory;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class NodeMap{
public:

	typedef std::map<geom::Coordinate*,Node*,geom::CoordinateLessThen> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef std::pair<geom::Coordinate*,Node*> pair;

	container nodeMap;
	const NodeFactory &nodeFact;
	// newNodeFact will be deleted by ~NodeMap
	NodeMap(const NodeFactory &newNodeFact);
	virtual ~NodeMap();
	Node* addNode(const geom::Coordinate& coord);
	Node* addNode(Node *n);
	void add(EdgeEnd *e);
	Node *find(const geom::Coordinate& coord) const;

	const_iterator begin() const { return nodeMap.begin(); }
	const_iterator end() const { return nodeMap.end(); }
	iterator begin() { return nodeMap.begin(); }
	iterator end() { return nodeMap.end(); }

	void getBoundaryNodes(int geomIndex,
		std::vector<Node*>&bdyNodes) const;

	std::string print() const;
};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/NodeMap.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_NODEMAP_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

