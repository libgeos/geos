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

#include <geos/operation/spanning/SpanningTreeGraph.h>
#include <geos/operation/spanning/SpanningTreeEdge.h>
#include <geos/planargraph/Node.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/geom/Curve.h>
#include <geos/geom/SimpleCurve.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>

using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos::operation
namespace spanning { // geos::operation::spanning

SpanningTreeGraph::~SpanningTreeGraph()
{
    // PlanarGraph doesn't seem to own the objects, so we must delete them.
    for (auto* de : newDirEdges) delete de;
    for (auto* e : newEdges) delete e;
    for (auto* n : newNodes) delete n;
}

Node*
SpanningTreeGraph::getNode(const Coordinate& coordinate)
{
    Node* node = findNode(coordinate);
    if (node == nullptr) {
        node = new Node(coordinate);
        add(node);
        newNodes.push_back(node);
    }
    return node;
}

void
SpanningTreeGraph::addEdge(const Curve* curve, std::size_t index)
{
    if (curve->isEmpty()) return;
    
    auto startPoint = curve->getStartPoint();
    auto endPoint = curve->getEndPoint();
    
    if (!startPoint || !endPoint) return;

    Coordinate startCoord = Coordinate(*(startPoint->getCoordinate()));
    Coordinate endCoord = Coordinate(*(endPoint->getCoordinate()));

    // We need a representative point for DirectedEdge (the "to" coordinate of the first segment)
    // to determine orientation.
    // For SimpleCurve (LineString, CircularString) we can get coordinates.
    // For CompoundCurve, we can get the first and last curves.

    Coordinate nextCoord;
    Coordinate prevCoord;
    bool foundNext = false;
    bool foundPrev = false;

    if (auto sc = dynamic_cast<const SimpleCurve*>(curve)) {
        const CoordinateSequence* coordinates = sc->getCoordinatesRO();
        std::size_t n = coordinates->size();
        
        // Find first point != startCoord
        std::size_t i = 1;
        while (i < n && coordinates->getAt(i).equals(startCoord)) {
            i++;
        }
        if (i < n) {
            nextCoord = coordinates->getAt(i);
            foundNext = true;
        }

        // Find last point != endCoord
        if (n > 0) {
            std::size_t j = n - 1;
            while (j > 0) {
                j--;
                if (!coordinates->getAt(j).equals(endCoord)) {
                    prevCoord = coordinates->getAt(j);
                    foundPrev = true;
                    break;
                }
            }
        }
    } else {
        // CompoundCurve
        std::size_t nCurves = curve->getNumCurves();
        for (std::size_t i = 0; i < nCurves; ++i) {
            auto subCurve = curve->getCurveN(i);
            const CoordinateSequence* coordinates = subCurve->getCoordinatesRO();
            for (std::size_t k = 0; k < coordinates->size(); ++k) {
                if (!coordinates->getAt(k).equals(startCoord)) {
                    nextCoord = coordinates->getAt(k);
                    foundNext = true;
                    break;
                }
            }
            if (foundNext) break;
        }

        if (nCurves > 0) {
            std::size_t i = nCurves - 1;
            while (true) {
                auto subCurve = curve->getCurveN(i);
                const CoordinateSequence* coordinates = subCurve->getCoordinatesRO();
                if (coordinates->size() > 0) {
                    std::size_t k = coordinates->size() - 1;
                    while (true) {
                        if (!coordinates->getAt(k).equals(endCoord)) {
                            prevCoord = coordinates->getAt(k);
                            foundPrev = true;
                            break;
                        }
                        if (k == 0) break;
                        k--;
                    }
                }
                if (foundPrev || i == 0) break;
                i--;
            }
        }
    }

    if (!foundNext || !foundPrev) return; // All points coincident

    Node* startNode = getNode(startCoord);
    Node* endNode = getNode(endCoord);

    // Create DirectedEdges
    DirectedEdge* dirEdge0 = new DirectedEdge(startNode, endNode, nextCoord, true);
    DirectedEdge* dirEdge1 = new DirectedEdge(endNode, startNode, prevCoord, false);
    
    newDirEdges.push_back(dirEdge0);
    newDirEdges.push_back(dirEdge1);

    Edge* edge = new SpanningTreeEdge(curve, index);
    edge->setDirectedEdges(dirEdge0, dirEdge1);
    
    newEdges.push_back(edge);

    add(edge);
}

} // namespace geos::operation::spanning
} // namespace geos::operation
} // namespace geos
