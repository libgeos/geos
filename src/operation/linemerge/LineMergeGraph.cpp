/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
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
 * Last port: operation/linemerge/LineMergeGraph.java r378 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/operation/linemerge/LineMergeGraph.h>
#include <geos/operation/linemerge/LineMergeEdge.h>
#include <geos/operation/linemerge/LineMergeDirectedEdge.h>
#include <geos/operation/valid/RepeatedPointRemover.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/planargraph/Node.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/util.h>
#include <geos/util/Assert.h>

#include <memory>
#include <optional>
#include <vector>


#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif


//using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace linemerge { // geos.operation.linemerge

LineMergeGraph::LineMergeGraph() = default;

std::optional<const CoordinateXY*>
getDirectionPoint(const Curve& curve, bool forward)
{
    if (curve.isEmpty()) {
        return std::nullopt;
    }

    if (curve.getGeometryTypeId() == GEOS_COMPOUNDCURVE) {
        if (forward) {
            for (std::size_t i = 0; i < curve.getNumCurves(); i++) {
                const auto dirPt = getDirectionPoint(*curve.getCurveN(i), true);
                if (dirPt.has_value()) {
                    return dirPt;
                }
            }
        } else {
            for (std::size_t i = curve.getNumCurves(); i > 0; i--) {
                const auto dirPt = getDirectionPoint(*curve.getCurveN(i - 1), false);
                if (dirPt.has_value()) {
                    return dirPt;
                }
            }
        }
    } else {
        // Although the method claims to be providing a direction point, we don't actually need to order our edges
        // around a node like we do for overlay. So we don't need to take the trouble of calculating a point along
        // a tangent a CircularString -- we can just use the first point that is not equal to the origin. This
        // also allows us to check the return value to make sure that the Curve is not collapsed to a single point.
        const CoordinateSequence& seq = *detail::down_cast<const SimpleCurve*>(&curve)->getCoordinatesRO();
        if (forward) {
            const CoordinateXY& startPt = seq.front<CoordinateXY>();
            for (std::size_t i = 1; i < seq.size(); i++) {
                const CoordinateXY& pt = seq.getAt<CoordinateXY>(i);
                if (!pt.equals2D(startPt)) {
                    return &pt;
                }
            }
        } else {
            const CoordinateXY& endPt = seq.back<CoordinateXY>();
            for (std::size_t i = seq.size(); i > 0; i--) {
                const CoordinateXY& pt = seq.getAt<CoordinateXY>(i - 1);
                if (!pt.equals2D(endPt)) {
                    return &pt;
                }
            }
        }
    }

    return std::nullopt;
}

void
LineMergeGraph::addEdge(const Curve* curve)
{
    if(curve->isEmpty()) {
        return;
    }

#if GEOS_DEBUG
    std::cerr << "Adding Curve " << curve->toString() << std::endl;
#endif

    const CoordinateXY& startCoordinate = curve->getStartCoordinate();
    const CoordinateXY& endCoordinate = curve->getEndCoordinate();

    planargraph::Node* startNode = getNode(startCoordinate);
    planargraph::Node* endNode = getNode(endCoordinate);
#if GEOS_DEBUG
    std::cerr << " startNode: " << *startNode << std::endl;
    std::cerr << " endNode: " << *endNode << std::endl;
#endif

    const auto dirPt0 = getDirectionPoint(*curve, true);

    if (!dirPt0.has_value()) {
        // edge has < 2 unique coordinates
        return;
    }

    newDirEdges.push_back(std::make_unique<LineMergeDirectedEdge>(startNode, endNode, *dirPt0.value(), true));
    auto* directedEdge0 = newDirEdges.back().get();

    const auto dirPt1 = getDirectionPoint(*curve, false);
    util::Assert::isTrue(dirPt1.has_value(), "dirPt1 should have been set");
    newDirEdges.push_back(std::make_unique<LineMergeDirectedEdge>(endNode, startNode, *dirPt1.value(), false));
    auto* directedEdge1 = newDirEdges.back().get();

    newEdges.push_back(std::make_unique<LineMergeEdge>(curve));
    planargraph::Edge* edge = newEdges.back().get();
    edge->setDirectedEdges(directedEdge0, directedEdge1);

#if GEOS_DEBUG
    std::cerr << " planargraph::Edge: " << *edge << std::endl;
#endif

    add(edge);

#if GEOS_DEBUG
    std::cerr << " After addition to the graph:" << std::endl;
    std::cerr << "  startNode: " << *startNode << std::endl;
    std::cerr << "  endNode: " << *endNode << std::endl;
#endif

}

planargraph::Node*
LineMergeGraph::getNode(const CoordinateXY& coordinate)
{
    planargraph::Node* node = findNode(coordinate);
    if(node == nullptr) {
        newNodes.push_back(std::make_unique<planargraph::Node>(coordinate));
        node = newNodes.back().get();
        add(node);
    }
    return node;
}

LineMergeGraph::~LineMergeGraph() = default;

} // namespace geos.operation.linemerge
} // namespace geos.operation
} // namespace geos
