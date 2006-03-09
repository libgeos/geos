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


#ifndef GEOS_GEOMGRAPH_EDGEEND_H
#define GEOS_GEOMGRAPH_EDGEEND_H

#include <string>

#include <geos/geom/Coordinate.h>  // for p0,p1

#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geomgraph {
		class Label;
		class Edge;
		class Node;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class EdgeEnd {
friend class Unload;
public:
	EdgeEnd();
	virtual ~EdgeEnd();
	EdgeEnd(Edge* newEdge, const geom::Coordinate& newP0,
		const geom::Coordinate& newP1);
	EdgeEnd(Edge* newEdge, const geom::Coordinate& newP0,
		const geom::Coordinate& newP1, Label* newLabel);
	virtual Edge* getEdge();
	virtual Label* getLabel();
	virtual geom::Coordinate& getCoordinate();
	virtual geom::Coordinate& getDirectedCoordinate();
	virtual int getQuadrant();
	virtual double getDx();
	virtual double getDy();
	virtual void setNode(Node* newNode);
	virtual Node* getNode();
	virtual int compareTo(const EdgeEnd *e) const;
	virtual int compareDirection(const EdgeEnd *e) const;
	virtual void computeLabel();
	virtual std::string print();
protected:
	Edge* edge;// the parent edge of this edge end
	Label* label;
	EdgeEnd(Edge* newEdge);
	virtual void init(const geom::Coordinate& newP0, const geom::Coordinate& newP1);
private:
	Node* node;          // the node this edge end originates at
	geom::Coordinate p0,p1;  // points of initial line segment
	double dx, dy;      // the direction vector for this edge from its starting point
	int quadrant;
};

struct EdgeEndLT {
	bool operator()(const EdgeEnd *s1, const EdgeEnd *s2) const {
		return s1->compareTo(s2)<0;
	}
};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/EdgeEnd.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGEEND_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

