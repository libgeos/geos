/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geomgraph/GeometryGraph.java rev. 1.5 (JTS-1.7)
 *
 * EXPOSED GEOS HEADER
 *
 **********************************************************************/

#ifndef GEOS_GEOMGRAPH_GEOMETRYGRAPH_INL
#define GEOS_GEOMGRAPH_GEOMETRYGRAPH_INL

#include <geos/geomgraph/GeometryGraph.h>

namespace geos {
namespace geomgraph { // geos::geomgraph

INLINE index::SegmentIntersector*
GeometryGraph::computeSelfNodes(
		algorithm::LineIntersector& li,
		bool computeRingSelfNodes)
{
	return computeSelfNodes(&li, computeRingSelfNodes);
}

INLINE void
GeometryGraph::getBoundaryNodes(std::vector<Node*>&bdyNodes)
{
	nodes->getBoundaryNodes(argIndex, bdyNodes);
}

INLINE const geom::Geometry*
GeometryGraph::getGeometry()
{
	return parentGeom;
}

INLINE 
GeometryGraph::~GeometryGraph()
{
}

INLINE
GeometryGraph::GeometryGraph(int newArgIndex,
		const geom::Geometry *newParentGeom)
	:
	PlanarGraph(),
	parentGeom(newParentGeom),
	useBoundaryDeterminationRule(true),
	argIndex(newArgIndex),
	hasTooFewPointsVar(false)
{
	if (parentGeom!=NULL) add(parentGeom);
}

INLINE
GeometryGraph::GeometryGraph()
	:
	PlanarGraph(),
	parentGeom(NULL),
	useBoundaryDeterminationRule(true),
	argIndex(-1),
	hasTooFewPointsVar(false)
{
}

} // namespace geos::geomgraph
} // namespace geos

#endif // GEOS_GEOMGRAPH_GEOMETRYGRAPH_INL
