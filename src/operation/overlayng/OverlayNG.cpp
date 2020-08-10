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

#include <geos/operation/overlayng/OverlayNG.h>

#include <geos/operation/overlayng/Edge.h>
#include <geos/operation/overlayng/EdgeNodingBuilder.h>
#include <geos/operation/overlayng/InputGeometry.h>
#include <geos/operation/overlayng/IntersectionPointBuilder.h>
#include <geos/operation/overlayng/LineBuilder.h>
#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/operation/overlayng/OverlayLabeller.h>
#include <geos/operation/overlayng/OverlayMixedPoints.h>
#include <geos/operation/overlayng/OverlayPoints.h>
#include <geos/operation/overlayng/OverlayUtil.h>
#include <geos/operation/overlayng/PolygonBuilder.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Location.h>
#include <geos/geom/Geometry.h>

#include <algorithm>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;


/*public static*/
bool
OverlayNG::isResultOfOpPoint(const OverlayLabel* label, int opCode)
{
    Location loc0 = label->getLocation(0);
    Location loc1 = label->getLocation(1);
    return isResultOfOp(opCode, loc0, loc1);
}

/*public static*/
bool
OverlayNG::isResultOfOp(int overlayOpCode, Location loc0, Location loc1)
{
    if (loc0 == Location::BOUNDARY) loc0 = Location::INTERIOR;
    if (loc1 == Location::BOUNDARY) loc1 = Location::INTERIOR;
    switch (overlayOpCode) {
        case INTERSECTION:
            return loc0 == Location::INTERIOR
                && loc1 == Location::INTERIOR;
        case UNION:
            return loc0 == Location::INTERIOR
                || loc1 == Location::INTERIOR;
        case DIFFERENCE:
            return loc0 == Location::INTERIOR
                && loc1 != Location::INTERIOR;
        case SYMDIFFERENCE:
            return   (loc0 == Location::INTERIOR && loc1 != Location::INTERIOR)
                  || (loc0 != Location::INTERIOR && loc1 == Location::INTERIOR);
    }
    return false;
}


/*public static*/
std::unique_ptr<Geometry>
OverlayNG::overlay(const Geometry* geom0, const Geometry* geom1,
        int opCode, const PrecisionModel* pm)
{
    OverlayNG ov(geom0, geom1, pm, opCode);
    return ov.getResult();
}

/*public static*/
std::unique_ptr<Geometry>
overlay(const Geometry* geom0, const Geometry* geom1,
        int opCode, const PrecisionModel* pm, noding::Noder* noder)
{
    OverlayNG ov(geom0, geom1, pm, opCode);
    ov.setNoder(noder);
    return ov.getResult();
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNG::overlay(const Geometry* geom0, const Geometry* geom1,
        int opCode, noding::Noder* noder)
{
    OverlayNG ov(geom0, geom1, nullptr, opCode);
    ov.setNoder(noder);
    return ov.getResult();
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNG::overlay(const Geometry* geom0, const Geometry* geom1, int opCode)
{
    OverlayNG ov(geom0, geom1, opCode);
    return ov.getResult();
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNG::geomunion(const Geometry* geom, const PrecisionModel* pm)
{
    OverlayNG ov(geom, pm);
    return ov.getResult();
}

/*public static*/
std::unique_ptr<Geometry>
OverlayNG::geomunion(const Geometry* geom, const PrecisionModel* pm, noding::Noder* noder)
{
    OverlayNG ov(geom, pm);
    ov.setNoder(noder);
    return ov.getResult();
}

/*public*/
std::unique_ptr<Geometry>
OverlayNG::getResult()
{
    if (OverlayUtil::isEmptyResult(opCode, inputGeom.getGeometry(0), inputGeom.getGeometry(1), pm)) {
        return createEmptyResult();
    }

    // special logic for Point-Point inputs
    if (inputGeom.isAllPoints()) {
        return OverlayPoints::overlay(opCode, inputGeom.getGeometry(0), inputGeom.getGeometry(1), pm);
    }

    // special logic for Point-nonPoint inputs
    if (! inputGeom.isSingle() &&  inputGeom.hasPoints()) {
        return OverlayMixedPoints::overlay(opCode, inputGeom.getGeometry(0), inputGeom.getGeometry(1), pm);
    }

    return computeEdgeOverlay();
}


/*private*/
std::unique_ptr<Geometry>
OverlayNG::computeEdgeOverlay()
{
    /**
     * Node the edges, using whatever noder is being used
     * Formerly in nodeEdges())
     */
    EdgeNodingBuilder nodingBuilder(pm, noder);

    if (isOptimized) {
        Envelope clipEnv;
        bool gotClipEnv = OverlayUtil::clippingEnvelope(opCode, &inputGeom, pm, clipEnv);
        if (gotClipEnv) {
            nodingBuilder.setClipEnvelope(&clipEnv);
        }
    }

    std::vector<Edge*> edges = nodingBuilder.build(
        inputGeom.getGeometry(0),
        inputGeom.getGeometry(1));

    /**
     * Record if an input geometry has collapsed.
     * This is used to avoid trying to locate disconnected edges
     * against a geometry which has collapsed completely.
     */
    inputGeom.setCollapsed(0, ! nodingBuilder.hasEdgesFor(0));
    inputGeom.setCollapsed(1, ! nodingBuilder.hasEdgesFor(1));

    /**
    * Inlined buildGraph() method here for memory purposes, so the
    * Edge* list allocated in the EdgeNodingBuilder survives
    * long enough to be copied into the OverlayGraph
    */
    // XXX sort the edges first
    // std::sort(edges.begin(), edges.end(), EdgeComparator);
    OverlayGraph graph;
    for (Edge* e : edges) {
        graph.addEdge(e);
    }

    if (isOutputNodedEdges) {
        return OverlayUtil::toLines(&graph, isOutputEdges, geomFact);
    }

    labelGraph(&graph);

    // std::cout << std::endl << graph << std::endl;

    if (isOutputEdges || isOutputResultEdges) {
        return OverlayUtil::toLines(&graph, isOutputEdges, geomFact);
    }

    return extractResult(opCode, &graph);
}

/*private*/
void
OverlayNG::labelGraph(OverlayGraph* graph)
{
    OverlayLabeller labeller(graph, &inputGeom);
    labeller.computeLabelling();
    labeller.markResultAreaEdges(opCode);
    labeller.unmarkDuplicateEdgesFromResultArea();
}


/*private*/
std::unique_ptr<Geometry>
OverlayNG::extractResult(int p_opCode, OverlayGraph* graph)
{
    //--- Build polygons
    std::vector<OverlayEdge*> resultAreaEdges = graph->getResultAreaEdges();
    PolygonBuilder polyBuilder(resultAreaEdges, geomFact);
    std::vector<std::unique_ptr<Polygon>> resultPolyList = polyBuilder.getPolygons();
    bool hasResultComponents = resultPolyList.size() > 0;

    //--- Build lines
    bool allowMixedIntResult = ! hasResultComponents || ALLOW_INT_MIXED_INT_RESULT;
    std::vector<std::unique_ptr<LineString>> resultLineList;
    if (p_opCode != INTERSECTION || allowMixedIntResult) {
        LineBuilder lineBuilder(&inputGeom, graph, hasResultComponents, p_opCode, geomFact);
        resultLineList = lineBuilder.getLines();
    }

    hasResultComponents = hasResultComponents || resultLineList.size() > 0;
    /**
     * Since operations with point inputs are handled elsewhere,
     * this only handles the case where non-point inputs
     * intersect in points.
     */
    std::vector<std::unique_ptr<Point>> resultPointList;
    allowMixedIntResult = ! hasResultComponents || ALLOW_INT_MIXED_INT_RESULT;
    if (opCode == INTERSECTION && allowMixedIntResult) {
        IntersectionPointBuilder pointBuilder(graph, geomFact);
        resultPointList = pointBuilder.getPoints();
    }

    if (resultPolyList.size() == 0 &&
        resultLineList.size() == 0 &&
        resultPointList.size() == 0)
    {
        return createEmptyResult();
    }

    std::unique_ptr<Geometry> resultGeom = OverlayUtil::createResultGeometry(resultPolyList, resultLineList, resultPointList, geomFact);
    return resultGeom;
}

/*private*/
std::unique_ptr<Geometry>
OverlayNG::createEmptyResult()
{
    return OverlayUtil::createEmptyResult(
                OverlayUtil::resultDimension(opCode,
                    inputGeom.getDimension(0),
                    inputGeom.getDimension(1)),
                geomFact);
}




} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
