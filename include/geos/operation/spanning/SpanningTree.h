/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Paul Ramsey
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Curve;
}
}

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

/** \brief
 * Constructs a Minimum Spanning Tree (MST) from a set of Curves.
 *
 * The input is a vector of Curves (LineString, CircularString, CompoundCurve)
 * which form the edges of a graph.
 * The nodes of the graph are the endpoints of the Curves.
 * Coincident endpoints are treated as the same node.
 *
 * The algorithm uses Kruskal's algorithm to find the MST.
 */
class GEOS_DLL SpanningTree {

public:
    
    /** \brief
     * Computes the Minimum Spanning Tree of the given Curves.
     *
     * @param curves Input vector of Curves.
     * @param result Output vector of size_t. 0 if the edge is not in the MST.
     *               Values > 0 indicate the component ID of the subtree the edge belongs to.
     *               The vector will be resized to match the input size.
     */
    static void mst(const std::vector<const geom::Curve*>& curves, std::vector<std::size_t>& result);

};

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
