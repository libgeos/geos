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
 * Last port: operation/IsSimpleOp.java rev. 1.17
 *
 **********************************************************************/

#ifndef GEOS_OPERATION_ISSIMPLEOP_H
#define GEOS_OPERATION_ISSIMPLEOP_H

#include <map>

// Forward declarations
namespace geos {
	namespace geom {
		class LineString;
		class MultiLineString;
		class MultiPoint;
		class Geometry;
		class Coordinate;
		struct CoordinateLessThen;
	}
	namespace geomgraph {
		class GeometryGraph;
	}
	namespace operation {
		class EndpointInfo;
	}
}


namespace geos {
namespace operation { // geos.operation

/** \brief
 * Tests whether a Geometry is simple.
 *
 * In general, the SFS specification of simplicity follows the rule:
 *
 *  - A Geometry is simple iff the only self-intersections are at
 *    boundary points.
 *
 * Simplicity is defined for each {@link Geometry} subclass as follows:
 * 
 *  - Valid polygonal geometries are simple by definition, so
 *    <code>isSimple</code> trivially returns true.
 *  - Linear geometries are simple iff they do not self-intersect at points
 *    other than boundary points.
 *  - Zero-dimensional geometries (points) are simple iff they have no
 *    repeated points.
 *  - Empty <code>Geometry</code>s are always simple
 *
 */
class IsSimpleOp {

public:

	IsSimpleOp();

	bool isSimple(const geom::LineString *geom);

	bool isSimple(const geom::MultiLineString *geom);

	/**
	 * A MultiPoint is simple iff it has no repeated points
	 */
	bool isSimple(const geom::MultiPoint *mp);

	bool isSimpleLinearGeometry(const geom::Geometry *geom);

private:

	/**
	 * For all edges, check if there are any intersections which are
	 * NOT at an endpoint.
	 * The Geometry is not simple if there are intersections not at
	 * endpoints.
	 */
	bool hasNonEndpointIntersection(geomgraph::GeometryGraph &graph);

	/**
	 * Tests that no edge intersection is the endpoint of a closed line.
	 * This ensures that closed lines are not touched at their endpoint,
	 * which is an interior point according to the Mod-2 rule
	 * To check this we compute the degree of each endpoint.
	 * The degree of endpoints of closed lines
	 * must be exactly 2.
	 */
	bool hasClosedEndpointIntersection(geomgraph::GeometryGraph &graph);

	/**
	 * Add an endpoint to the map, creating an entry for it if none exists
	 */
	void addEndpoint(std::map<const geom::Coordinate*, EndpointInfo*,
			geom::CoordinateLessThen>&endPoints,
			const geom::Coordinate *p, bool isClosed);
};

} // namespace geos.operation
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/15 18:59:33  strk
 * Bug #62: 'struct' CoordinateLessThen in forward declaration
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

