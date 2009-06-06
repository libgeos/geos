/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OP_LINEMERGE_LINEMERGEGRAPH_H
#define GEOS_OP_LINEMERGE_LINEMERGEGRAPH_H

#include <geos/export.h>

#include <geos/planargraph/PlanarGraph.h> // for inheritance

#include <vector>

// Forward declarations 
namespace geos {
	namespace geom { 
		class LineString;
		class Coordinate;
	}
	namespace planargraph { 
		class Node;
		class Edge;
		class DirectedEdge;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

/** \brief
 * A planar graph of edges that is analyzed to sew the edges together.
 *
 * The <code>marked</code> flag on planargraph::Edge
 * and planargraph::Node indicates whether they have been
 * logically deleted from the graph.
 */
class GEOS_DLL LineMergeGraph: public planargraph::PlanarGraph {

private:

	planargraph::Node* getNode(const geom::Coordinate &coordinate);

	std::vector<planargraph::Node*> newNodes;

	std::vector<planargraph::Edge*> newEdges;

	std::vector<planargraph::DirectedEdge*> newDirEdges;

public:

	/**
	 * Adds an Edge, DirectedEdges, and Nodes for the given
	 * LineString representation of an edge. 
	 */
	void addEdge(const geom::LineString *lineString);

	~LineMergeGraph();
};
} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_LINEMERGE_LINEMERGEGRAPH_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 10:13:53  strk
 * opLinemerge.h split
 *
 **********************************************************************/
