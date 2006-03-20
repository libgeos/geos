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

#ifndef GEOS_OP_CONSISTENTAREATESTER_H
#define GEOS_OP_CONSISTENTAREATESTER_H

#include <geos/geom/Coordinate.h> // for composition

// Forward declarations
namespace geos {
	namespace algorithm {
		class LineIntersector;
	}
	namespace geomgraph {
		class GeometryGraph;
	}
	namespace operation {
		namespace relate {
			class RelateNodeGraph;
		}
	}
}

namespace geos {
namespace operation { // geos::operation
namespace valid { // geos::operation::valid

/** \brief
 * Checks that a {@link geomgraph::GeometryGraph} representing an area
 * (a {@link Polygon} or {@link MultiPolygon} )
 * is consistent with the SFS semantics for area geometries.
 *
 * Checks include:
 * <ul>
 * <li>testing for rings which self-intersect (both properly
 * and at nodes)
 * <li>testing for duplicate rings
 * </ul>
 * If an inconsistency if found the location of the problem
 * is recorded.
 */
class ConsistentAreaTester {
private:

	algorithm::LineIntersector *li;

	geomgraph::GeometryGraph *geomGraph;

	relate::RelateNodeGraph *nodeGraph;

	/// the intersection point found (if any)
	geom::Coordinate invalidPoint;

	/**
	 * Check all nodes to see if their labels are consistent.
	 * If any are not, return false
	 */
	bool isNodeEdgeAreaLabelsConsistent();

public:

	ConsistentAreaTester(geomgraph::GeometryGraph *newGeomGraph);

	~ConsistentAreaTester();

	/**
	 * @return the intersection point, or <code>null</code> if none was found
	 */
	geom::Coordinate& getInvalidPoint();

	bool isNodeConsistentArea();

	/**
	 * Checks for two duplicate rings in an area.
	 * Duplicate rings are rings that are topologically equal
	 * (that is, which have the same sequence of points up to point order).
	 * If the area is topologically consistent (determined by calling the
	 * <code>isNodeConsistentArea</code>,
	 * duplicate rings can be found by checking for EdgeBundles which contain
	 * more than one geomgraph::EdgeEnd.
	 * (This is because topologically consistent areas cannot have two rings sharing
	 * the same line segment, unless the rings are equal).
	 * The start point of one of the equal rings will be placed in
	 * invalidPoint.
	 *
	 * @return true if this area Geometry is topologically consistent but has two duplicate rings
	 */
	bool hasDuplicateRings();
};



} // namespace geos::operation::valid
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_CONSISTENTAREATESTER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 **********************************************************************/

