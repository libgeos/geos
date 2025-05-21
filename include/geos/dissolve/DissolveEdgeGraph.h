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

#pragma once

#include <geos/edgegraph/EdgeGraph.h>
#include <geos/export.h>
#include <geos/dissolve/DissolveHalfEdge.h>

#include <deque>

// Forward declarations
namespace geos {
namespace geom {
    class CoordinateXYZM;
}
namespace edgegraph {
    class HalfEdge;
}
namespace dissolve {
    // class DissolveHalfEdge;
}
}


namespace geos {      // geos.
namespace dissolve {  // geos.dissolve


class GEOS_DLL DissolveEdgeGraph : public edgegraph::EdgeGraph {


private:

    std::deque<DissolveHalfEdge> dhEdges;

public:

    edgegraph::HalfEdge* createEdge(const geom::CoordinateXYZM& p0) override;

    DissolveEdgeGraph() {};
    ~DissolveEdgeGraph() {};

    /**
     * Disable copy construction and assignment. Needed to make this
     * class compile under MSVC, because it has a vector<unique_ptr>
     * as a member. (See https://stackoverflow.com/q/29565299)
     */
    DissolveEdgeGraph(const DissolveEdgeGraph&) = delete;
    DissolveEdgeGraph& operator=(const DissolveEdgeGraph&) = delete;

};

} // namespace geos.dissolve
} // namespace geos

