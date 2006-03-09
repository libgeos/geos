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


#ifndef GEOS_GEOMGRAPH_DIRECTEDEDGE_H
#define GEOS_GEOMGRAPH_DIRECTEDEDGE_H

#include <string>

#include <geos/geomgraph/EdgeEnd.h> // for inheritance

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geomgraph {
		class Edge;
		class EdgeRing;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class DirectedEdge: public EdgeEnd{
public:
	static int depthFactor(int currLocation, int nextLocation);
	//DirectedEdge();	
	//virtual ~DirectedEdge();	
	DirectedEdge(Edge *newEdge, bool newIsForward);
	Edge* getEdge();
	void setInResult(bool newIsInResult);
	bool isInResult();
	bool isVisited();
	void setVisited(bool newIsVisited);
	void setEdgeRing(EdgeRing *newEdgeRing);
	EdgeRing* getEdgeRing();
	void setMinEdgeRing(EdgeRing *newMinEdgeRing);
	EdgeRing* getMinEdgeRing();
	int getDepth(int position);
	void setDepth(int position, int newDepth);
	int getDepthDelta();
	void setVisitedEdge(bool newIsVisited);

	/**
	 * Each Edge gives rise to a pair of symmetric DirectedEdges,
	 * in opposite directions.
	 * @return the DirectedEdge for the same Edge but in the
	 *         opposite direction
	 */
	DirectedEdge* getSym();

	bool isForward();
	void setSym(DirectedEdge *de);
	DirectedEdge* getNext();
	void setNext(DirectedEdge *newNext);
	DirectedEdge* getNextMin();
	void setNextMin(DirectedEdge *newNextMin);
	bool isLineEdge();
	bool isInteriorAreaEdge();
	void setEdgeDepths(int position, int newDepth);
	void OLDsetEdgeDepths(int position, int newDepth);
	std::string print();
	std::string printEdge();
protected:
	bool isForwardVar;

private:
	bool isInResultVar;
	bool isVisitedVar;
	DirectedEdge *sym; // the symmetric edge
	DirectedEdge *next;  // the next edge in the edge ring for the polygon containing this edge
	DirectedEdge *nextMin;  // the next edge in the MinimalEdgeRing that contains this edge
	EdgeRing *edgeRing;  // the EdgeRing that this edge is part of
	EdgeRing *minEdgeRing;  // the MinimalEdgeRing that this edge is part of
	/**
	 * The depth of each side (position) of this edge.
	 * The 0 element of the array is never used.
	 */
	int depth[3];
	void computeDirectedLabel();
};

} // namespace geos.geomgraph
} // namespace geos

#ifdef USE_INLINE
# include "geos/geomgraph/DirectedEdge.inl"
#endif

#endif // ifndef GEOS_GEOMGRAPH_DIRECTEDEDGE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

