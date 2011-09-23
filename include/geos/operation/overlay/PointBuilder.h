/**********************************************************************
 * $Id: PointBuilder.h 2757 2009-12-01 15:39:41Z mloskot $
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
 ***********************************************************************
 *
 * Last port: operation/overlay/PointBuilder.java rev. 1.16 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_OP_OVERLAY_POINTBUILDER_H
#define GEOS_OP_OVERLAY_POINTBUILDER_H

#include <geos/export.h>

#include <geos/geom/GeometryFactory.h> // for inlines
#include <geos/operation/overlay/OverlayOp.h> // for OpCode enum
#include <geos/util.h>

#include <vector>

// Forward declarations
namespace geos {
	namespace geom {
		class GeometryFactory;
		class Point;
	}
	namespace geomgraph {
		class Node;
	}
	namespace algorithm {
		class PointLocator;
	}
	namespace operation {
		namespace overlay {
			class OverlayOp;
		}
	}
}

namespace geos {
namespace operation { // geos::operation
namespace overlay { // geos::operation::overlay

/** \brief
 * Constructs geom::Point s from the nodes of an overlay graph.
 */
class GEOS_DLL PointBuilder {
private:

	OverlayOp *op;
	const geom::GeometryFactory *geometryFactory;
	void extractNonCoveredResultNodes(OverlayOp::OpCode opCode);

	/*
	 * Converts non-covered nodes to Point objects and adds them to
	 * the result.
	 *
	 * A node is covered if it is contained in another element Geometry
	 * with higher dimension (e.g. a node point might be contained in
	 * a polygon, in which case the point can be eliminated from
	 * the result).
	 *
	 * @param n the node to test
	 */
	void filterCoveredNodeToPoint(const geomgraph::Node *);

	/// Allocated a construction time, but not owned.
	/// Make sure you take ownership of it, getting 
	/// it from build()
	std::vector<geom::Point*> *resultPointList;

public:

	PointBuilder(OverlayOp *newOp,
			const geom::GeometryFactory *newGeometryFactory,
			algorithm::PointLocator *newPtLocator=NULL)
		:
		op(newOp),
		geometryFactory(newGeometryFactory),
		resultPointList(new std::vector<geom::Point *>())
	{
        ::geos::ignore_unused_variable_warning(newPtLocator);
    }

	/**
	 * @return a list of the Points in the result of the specified
	 * overlay operation
	 */
	std::vector<geom::Point*>* build(OverlayOp::OpCode opCode);
};


} // namespace geos::operation::overlay
} // namespace geos::operation
} // namespace geos

#endif // ndef GEOS_OP_OVERLAY_POINTBUILDER_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/06/05 15:36:34  strk
 * Given OverlayOp funx code enum a name and renamed values to have a lowercase prefix. Drop all of noding headers from installed header set.
 *
 * Revision 1.1  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 **********************************************************************/

