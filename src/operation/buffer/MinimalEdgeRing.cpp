/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 *
 **********************************************************************/

#include <geos/operation/buffer/MinimalEdgeRing.h>
#include <geos/geomgraph/EdgeRing.h>


namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer


MinimalEdgeRing::MinimalEdgeRing(geomgraph::DirectedEdge* start,
                                 const geom::GeometryFactory* p_geometryFactory)
    :
    geomgraph::EdgeRing(start, p_geometryFactory)
{
    computePoints(start);
    computeRing();
}


} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

