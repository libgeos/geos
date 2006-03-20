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


#ifndef GEOS_GEOMGRAPH_EDGERING_H
#define GEOS_GEOMGRAPH_EDGERING_H

#include <vector>


#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom {
		class GeometryFactory;
		class LinearRing;
		class Polygon;
		class Coordinate;
		class CoordinateSequence;
	}
	namespace geomgraph {
		class DirectedEdge;
		class Label;
		class Edge;
	}
}

namespace geos {
namespace geomgraph { // geos.geomgraph

class EdgeRing {
public:

	EdgeRing(DirectedEdge *newStart, const geom::GeometryFactory *newGeometryFactory);

	virtual ~EdgeRing();
	bool isIsolated();
	bool isHole();

	/*
	 * Return a pointer to the LinearRing owned by
	 * this object. Make a copy if you need it beyond
	 * this objects's lifetime.
	 */
	geom::LinearRing* getLinearRing();

	Label* getLabel();
	bool isShell();
	EdgeRing *getShell();
	void setShell(EdgeRing *newShell);
	void addHole(EdgeRing *edgeRing);

	/**
	 * Return a Polygon copying coordinates from this
	 * EdgeRing and its holes. Caller must remember
	 * to delete the result
	 */
	geom::Polygon* toPolygon(const geom::GeometryFactory* geometryFactory);

	void computeRing();
	virtual DirectedEdge* getNext(DirectedEdge *de)=0;
	virtual void setEdgeRing(DirectedEdge *de, EdgeRing *er)=0;
	std::vector<DirectedEdge*>* getEdges();
	int getMaxNodeDegree();
	void setInResult();
	bool containsPoint(const geom::Coordinate& p);
protected:
	DirectedEdge *startDe; // the directed edge which starts the list of edges for this EdgeRing
	const geom::GeometryFactory *geometryFactory;
	void computePoints(DirectedEdge *newStart);
	void mergeLabel(Label *deLabel);
	void mergeLabel(Label *deLabel, int geomIndex);
	void addPoints(Edge *edge, bool isForward, bool isFirstEdge);
	std::vector<EdgeRing*> holes; // a list of EdgeRings which are holes in this EdgeRing
private:
	int maxNodeDegree;
	std::vector<DirectedEdge*> edges; // the DirectedEdges making up this EdgeRing
	geom::CoordinateSequence* pts;
	Label* label; // label stores the locations of each geometry on the face surrounded by this ring
	geom::LinearRing *ring;  // the ring created for this EdgeRing
	bool isHoleVar;
	EdgeRing *shell;   // if non-null, the ring is a hole and this EdgeRing is its containing shell
	void computeMaxNodeDegree();
};

} // namespace geos.geomgraph
} // namespace geos

//#ifdef USE_INLINE
//# include "geos/geomgraph/EdgeRing.inl"
//#endif

#endif // ifndef GEOS_GEOMGRAPH_EDGERING_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/03/20 12:32:57  strk
 * Added note about responsibility of return from ::toPolygon
 *
 * Revision 1.2  2006/03/15 17:17:41  strk
 * Added missing forward declarations
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

