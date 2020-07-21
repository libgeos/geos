/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
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
#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class Point;
}
namespace operation {
namespace overlayng {
class OverlayEdge;
class OverlayGraph;
class OverlayLabel;
}
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

/**
 * Extracts Point resultants from an overlay graph
 * created by an Intersection operation
 * between non-Point inputs.
 * Points may be created during intersection
 * if lines or areas touch one another at single points.
 * Intersection is the only overlay operation which can
 * result in Points from non-Point inputs.
 * <p>
 * Overlay operations where one or more inputs
 * are Points are handled via a different code path.
 *
 *
 * @author Martin Davis
 *
 * @see OverlayPoints
 *
 */
class GEOS_DLL IntersectionPointBuilder {

private:

    // Members
    OverlayGraph* graph;
    const geom::GeometryFactory* geometryFactory;
    std::vector<std::unique_ptr<geom::Point>> points;

    // Methods
    void addResultPoints();

    /**
    * Tests if a node is a result point.
    * This is the case if the node is incident on edges from both
    * inputs, and none of the edges are themselves in the result.
    */
    bool isResultPoint(OverlayEdge* nodeEdge) const;
    bool isEdgeOf(const OverlayLabel* label, int i) const;



public:


    IntersectionPointBuilder(OverlayGraph* p_graph, const geom::GeometryFactory* geomFact)
        : graph(p_graph)
        , geometryFactory(geomFact) {}

    std::vector<std::unique_ptr<geom::Point>> getPoints();



};


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos

