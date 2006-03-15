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
 * Only Geometry whose definition allows them
 * to be simple or non-simple are tested.  (E.g. Polygons must be simple
 * by definition, so no test is provided.  To test whether a given Polygon is valid,
 * use <code>Geometry#isValid</code>)
 *
 */
class IsSimpleOp {
public:
	IsSimpleOp();
	bool isSimple(const geom::LineString *geom);
	bool isSimple(const geom::MultiLineString *geom);
	bool isSimple(const geom::MultiPoint *mp);
	bool isSimpleLinearGeometry(const geom::Geometry *geom);
private:
	bool hasNonEndpointIntersection(geomgraph::GeometryGraph &graph);
	bool hasClosedEndpointIntersection(geomgraph::GeometryGraph &graph);
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

