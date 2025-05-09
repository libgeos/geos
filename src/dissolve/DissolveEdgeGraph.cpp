/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2025 Martin Davis
 * Copyright (C) 2025 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/dissolve/DissolveEdgeGraph.h>

#include <geos/edgegraph/HalfEdge.h>
#include <geos/geom/Coordinate.h>


using geos::edgegraph::HalfEdge;
using geos::geom::CoordinateXYZM;


namespace geos {      // geos
namespace dissolve {  // geos.dissolve


HalfEdge*
DissolveEdgeGraph::createEdge(const CoordinateXYZM& p0)
{
    dhEdges.emplace_back(p0);
    return &(dhEdges.back());
}


} // namespace geos.dissolve
} // namespace geos


