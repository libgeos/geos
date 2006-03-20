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

#ifndef GEOS_OP_CONNECTEDINTERIORTESTER_H
#define GEOS_OP_CONNECTEDINTERIORTESTER_H

#include <geos/geom/Coordinate.h> // for composition

#include <vector>

// Forward declarations
namespace geos {
	namespace geom {
		//class Coordinate;
		class Geometry;
		class CoordinateSequence;
		class GeometryFactory;
		class LineString;
	}
	namespace geomgraph {
		class GeometryGraph;
		class PlanarGraph;
		class EdgeRing;
		class DirectedEdge;
		class EdgeEnd;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace valid { // geos::operation::valid

/** \brief
 * This class tests that the interior of an area Geometry
 * (Polygon or MultiPolygon)
 * is connected. 
 *
 * An area Geometry is invalid if the interior is disconnected.
 * This can happen if:
 * 
 * - one or more holes either form a chain touching the shell at two places
 * - one or more holes form a ring around a portion of the interior
 * 
 * If an inconsistency if found the location of the problem
 * is recorded.
 */
class ConnectedInteriorTester {
public:
	ConnectedInteriorTester(geomgraph::GeometryGraph &newGeomGraph);
	~ConnectedInteriorTester();
	geom::Coordinate& getCoordinate();
	bool isInteriorsConnected();
	static const geom::Coordinate& findDifferentPoint(
			const geom::CoordinateSequence *coord,
			const geom::Coordinate& pt);

private:

	geom::GeometryFactory *geometryFactory;

	geomgraph::GeometryGraph &geomGraph;

	/// Save a coordinate for any disconnected interior found
	/// the coordinate will be somewhere on the ring surrounding
	/// the disconnected interior
	geom::Coordinate disconnectedRingcoord;

	void setInteriorEdgesInResult(geomgraph::PlanarGraph &graph);

	
	/**
	 * Form DirectedEdges in graph into Minimal EdgeRings.
	 * (Minimal Edgerings must be used, because only they are guaranteed to provide
	 * a correct isHole computation)
	 */
	std::vector<geomgraph::EdgeRing*>* buildEdgeRings(
			std::vector<geomgraph::EdgeEnd*> *dirEdges);

	/**
	 * Mark all the edges for the edgeRings corresponding to the shells
	 * of the input polygons.  Note only ONE ring gets marked for each shell.
	 */
	void visitShellInteriors(const geom::Geometry *g, geomgraph::PlanarGraph &graph);

	void visitInteriorRing(const geom::LineString *ring, geomgraph::PlanarGraph &graph);

	/**
	 * Check if any shell ring has an unvisited edge.
	 * A shell ring is a ring which is not a hole and which has the interior
	 * of the parent area on the RHS.
	 * (Note that there may be non-hole rings with the interior on the LHS,
	 * since the interior of holes will also be polygonized into CW rings
	 * by the linkAllDirectedEdges() step)
	 *
	 * @return true if there is an unvisited edge in a non-hole ring
	 */
	bool hasUnvisitedShellEdge(std::vector<geomgraph::EdgeRing*> *edgeRings);

protected:

	void visitLinkedDirectedEdges(geomgraph::DirectedEdge *start);
};


} // namespace geos::operation::valid
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_CONNECTEDINTERIORTESTER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 **********************************************************************/

