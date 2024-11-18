/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Location.h>
#include <geos/geom/Position.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/operation/relateng/TopologyComputer.h>
#include <geos/operation/relateng/TopologyPredicate.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <geos/operation/relateng/RelateNode.h>
#include <geos/operation/relateng/NodeSection.h>
#include <geos/util/IllegalStateException.h>
#include <sstream>


using geos::algorithm::PolygonNodeTopology;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::Dimension;
using geos::geom::Location;
using geos::geom::Position;
using geos::util::IllegalStateException;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* private */
void
TopologyComputer::initExteriorDims()
{
    int dimRealA = geomA.getDimensionReal();
    int dimRealB = geomB.getDimensionReal();

    /**
     * For P/L case, P exterior intersects L interior
     */
    if (dimRealA == Dimension::P && dimRealB == Dimension::L) {
        updateDim(Location::EXTERIOR, Location::INTERIOR, Dimension::L);
    }
    else if (dimRealA == Dimension::L && dimRealB == Dimension::P) {
        updateDim(Location::INTERIOR, Location::EXTERIOR, Dimension::L);
    }
    /**
     * For P/A case, the Area Int and Bdy intersect the Point exterior.
     */
    else if (dimRealA == Dimension::P && dimRealB == Dimension::A) {
        updateDim(Location::EXTERIOR, Location::INTERIOR, Dimension::A);
        updateDim(Location::EXTERIOR, Location::BOUNDARY, Dimension::L);
    }
    else if (dimRealA == Dimension::A && dimRealB == Dimension::P) {
        updateDim(Location::INTERIOR, Location::EXTERIOR, Dimension::A);
        updateDim(Location::BOUNDARY, Location::EXTERIOR, Dimension::L);
    }
    else if (dimRealA == Dimension::L && dimRealB == Dimension::A) {
        updateDim(Location::EXTERIOR, Location::INTERIOR, Dimension::A);
     }
    else if (dimRealA == Dimension::A && dimRealB == Dimension::L) {
        updateDim(Location::INTERIOR, Location::EXTERIOR, Dimension::A);
    }
    //-- cases where one geom is EMPTY
    else if (dimRealA == Dimension::False || dimRealB == Dimension::False) {
        if (dimRealA != Dimension::False) {
            initExteriorEmpty(RelateGeometry::GEOM_A);
        }
        if (dimRealB != Dimension::False) {
            initExteriorEmpty(RelateGeometry::GEOM_B);
        }
    }
}


/* private */
void
TopologyComputer::initExteriorEmpty(bool geomNonEmpty)
{
    int dimNonEmpty = getDimension(geomNonEmpty);
    switch (dimNonEmpty) {
        case Dimension::P:
            updateDim(geomNonEmpty, Location::INTERIOR, Location::EXTERIOR, Dimension::P);
            break;
        case Dimension::L:
            if (getGeometry(geomNonEmpty).hasBoundary()) {
                updateDim(geomNonEmpty, Location::BOUNDARY, Location::EXTERIOR, Dimension::P);
            }
            updateDim(geomNonEmpty, Location::INTERIOR, Location::EXTERIOR, Dimension::L);
            break;
        case Dimension::A:
            updateDim(geomNonEmpty, Location::BOUNDARY, Location::EXTERIOR, Dimension::L);
            updateDim(geomNonEmpty, Location::INTERIOR, Location::EXTERIOR, Dimension::A);
            break;
    }
}


/* public */
bool
TopologyComputer::isAreaArea() const
{
    return getDimension(RelateGeometry::GEOM_A) == Dimension::A
        && getDimension(RelateGeometry::GEOM_B) == Dimension::A;
}


/* public */
int
TopologyComputer::getDimension(bool isA) const
{
    return getGeometry(isA).getDimension();
}


/* public */
bool
TopologyComputer::isSelfNodingRequired() const
{
    if (predicate.requireSelfNoding()) {
        if (geomA.isSelfNodingRequired() ||
            geomB.isSelfNodingRequired())
        return true;
    }
    return false;
}


/* public */
bool
TopologyComputer::isExteriorCheckRequired(bool isA) const
{
    return predicate.requireExteriorCheck(isA);
}

// static char
// toSymbol(Location loc) {
//     switch (loc) {
//         case Location::NONE: return '-';
//         case Location::INTERIOR: return 'I';
//         case Location::BOUNDARY: return 'B';
//         case Location::EXTERIOR: return 'E';
//     }
//     return ' ';
// }

/* private */
void
TopologyComputer::updateDim(Location locA, Location locB, int dimension)
{
    //std::cout << toSymbol(locA) << toSymbol(locB) << " <- " << dimension << std::endl;
    predicate.updateDimension(locA, locB, dimension);
}

/* private */
void
TopologyComputer::updateDim(bool isAB, Location loc1, Location loc2, int dimension)
{
    if (isAB) {
        updateDim(loc1, loc2, dimension);
    }
    else {
        // is ordered BA
        updateDim(loc2, loc1, dimension);
    }
}


/* public */
bool
TopologyComputer::isResultKnown() const
{
    return predicate.isKnown();
}


/* public */
bool
TopologyComputer::getResult() const
{
    return predicate.value();
}


/* public */
void
TopologyComputer::finish()
{
    predicate.finish();
}


/* private */
NodeSections *
TopologyComputer::getNodeSections(const CoordinateXY& nodePt)
{
    NodeSections* ns;
    auto result = nodeMap.find(nodePt);
    if (result == nodeMap.end()) {
        ns = new NodeSections(&nodePt);
        nodeSectionsStore.emplace_back(ns);
        nodeMap[nodePt] = ns;
    }
    else {
        ns = result->second;
    }
    return ns;
}


/* public */
void
TopologyComputer::addIntersection(NodeSection* a, NodeSection* b)
{
    // add edges to node to allow full topology evaluation later
    // we run this first (unlike JTS) in case the subsequent test throws
    // an exception and the NodeSection pointers are not correctly
    // saved in the memory managed store on the NodeSections, causing
    // a small memory leak
    addNodeSections(a, b);

    if (! a->isSameGeometry(b)) {
        updateIntersectionAB(a, b);
    }
}


/* private */
void
TopologyComputer::updateIntersectionAB(const NodeSection* a, const NodeSection* b)
{
    if (NodeSection::isAreaArea(*a, *b)) {
        updateAreaAreaCross(a, b);
    }
    updateNodeLocation(a, b);
}


/* private */
void
TopologyComputer::updateAreaAreaCross(const NodeSection* a, const NodeSection* b)
{
    bool isProper = NodeSection::isProper(*a, *b);
    if (isProper || PolygonNodeTopology::isCrossing(&(a->nodePt()),
        a->getVertex(0), a->getVertex(1),
        b->getVertex(0), b->getVertex(1)))
    {
        updateDim(Location::INTERIOR, Location::INTERIOR, Dimension::A);
    }
}


/* private */
void
TopologyComputer::updateNodeLocation(const NodeSection* a, const NodeSection* b)
{
    const CoordinateXY& pt = a->nodePt();
    Location locA = geomA.locateNode(&pt, a->getPolygonal());
    Location locB = geomB.locateNode(&pt, b->getPolygonal());
    updateDim(locA, locB, Dimension::P);
}

/* private */
void
TopologyComputer::addNodeSections(NodeSection* ns0, NodeSection* ns1)
{
    NodeSections* sections = getNodeSections(ns0->nodePt());
    sections->addNodeSection(ns0);
    sections->addNodeSection(ns1);
}

/* public */
void
TopologyComputer::addPointOnPointInterior(const CoordinateXY* pt)
{
    (void)pt;
    updateDim(Location::INTERIOR, Location::INTERIOR, Dimension::P);
}

/* public */
void
TopologyComputer::addPointOnPointExterior(bool isGeomA, const CoordinateXY* pt)
{
    (void)pt;
    updateDim(isGeomA, Location::INTERIOR, Location::EXTERIOR, Dimension::P);
}

/* public */
void
TopologyComputer::addPointOnGeometry(bool isPointA, Location locTarget, int dimTarget, const CoordinateXY* pt)
{
    (void)pt;
    //-- update entry for Point interior
    updateDim(isPointA, Location::INTERIOR, locTarget, Dimension::P);

    //-- an empty geometry has no points to infer entries from
    if (getGeometry(! isPointA).isEmpty())
      return;

    switch (dimTarget) {
    case Dimension::P:
        return;
    case Dimension::L:
        /**
         * Because zero-length lines are handled,
         * a point lying in the exterior of the line target
         * may imply either P or L for the Exterior interaction
         */
        //TODO: determine if effective dimension of linear target is L?
        //updateDim(isGeomA, Location::EXTERIOR, locTarget, Dimension::P);
        return;
    case Dimension::A:
        /**
         * If a point intersects an area target, then the area interior and boundary
         * must extend beyond the point and thus interact with its exterior.
         */
        updateDim(isPointA, Location::EXTERIOR, Location::INTERIOR, Dimension::A);
        updateDim(isPointA, Location::EXTERIOR, Location::BOUNDARY, Dimension::L);
        return;
    }
    throw IllegalStateException("Unknown target dimension: " + std::to_string(dimTarget));
}

/* public */
void
TopologyComputer::addLineEndOnGeometry(bool isLineA, Location locLineEnd, Location locTarget, int dimTarget, const CoordinateXY* pt)
{
    (void)pt;

    //-- record topology at line end point
    updateDim(isLineA, locLineEnd, locTarget, Dimension::P);

    //-- an empty geometry has no points to infer entries from
    if (getGeometry(! isLineA).isEmpty())
      return;
      
    //-- Line and Area targets may have additional topology
    switch (dimTarget) {
    case Dimension::P:
        return;
    case Dimension::L:
        addLineEndOnLine(isLineA, locLineEnd, locTarget, pt);
        return;
    case Dimension::A:
        addLineEndOnArea(isLineA, locLineEnd, locTarget, pt);
        return;
    }
    throw IllegalStateException("Unknown target dimension: " + std::to_string(dimTarget));
}


/* private */
void
TopologyComputer::addLineEndOnLine(bool isLineA, Location locLineEnd, Location locLine, const CoordinateXY* pt)
{
    (void)pt;
    (void)locLineEnd;
    /**
     * When a line end is in the EXTERIOR of a Line,
     * some length of the source Line INTERIOR
     * is also in the target Line EXTERIOR.
     * This works for zero-length lines as well.
     */
    if (locLine == Location::EXTERIOR) {
        updateDim(isLineA, Location::INTERIOR, Location::EXTERIOR, Dimension::L);
    }
}


/* private */
void
TopologyComputer::addLineEndOnArea(bool isLineA, Location locLineEnd, Location locArea, const CoordinateXY* pt)
{
    (void)pt;
    (void)locLineEnd;
    if (locArea != Location::BOUNDARY) {
        /**
         * When a line end is in an Area INTERIOR or EXTERIOR
         * some length of the source Line Interior
         * AND the Exterior of the line
         * is also in that location of the target.
         * NOTE: this assumes the line end is NOT also in an Area of a mixed-dim GC
         */
        //TODO: handle zero-length lines?
        updateDim(isLineA, Location::INTERIOR, locArea, Dimension::L);
        updateDim(isLineA, Location::EXTERIOR, locArea, Dimension::A);
    }
}


/* public */
void
TopologyComputer::addAreaVertex(bool isAreaA, Location locArea, Location locTarget, int dimTarget, const CoordinateXY* pt)
{
    (void)pt;
    if (locTarget == Location::EXTERIOR) {
        updateDim(isAreaA, Location::INTERIOR, Location::EXTERIOR, Dimension::A);
        /**
         * If area vertex is on Boundary further topology can be deduced
         * from the neighbourhood around the boundary vertex.
         * This is always the case for polygonal geometries.
         * For GCs, the vertex may be either on boundary or in interior
         * (i.e. of overlapping or adjacent polygons)
         */
        if (locArea == Location::BOUNDARY) {
            updateDim(isAreaA, Location::BOUNDARY, Location::EXTERIOR, Dimension::L);
            updateDim(isAreaA, Location::EXTERIOR, Location::EXTERIOR, Dimension::A);
        }
        return;
    }

    switch (dimTarget) {
        case Dimension::P:
            addAreaVertexOnPoint(isAreaA, locArea, pt);
            return;
        case Dimension::L:
            addAreaVertexOnLine(isAreaA, locArea, locTarget, pt);
            return;
        case Dimension::A:
            addAreaVertexOnArea(isAreaA, locArea, locTarget, pt);
            return;
    }
    throw IllegalStateException("Unknown target dimension: " + std::to_string(dimTarget));
}


/* private */
void
TopologyComputer::addAreaVertexOnPoint(bool isAreaA, Location locArea, const CoordinateXY* pt)
{
    (void)pt;
    //-- Assert: locArea != EXTERIOR
    //-- Assert: locTarget == INTERIOR
    /**
     * The vertex location intersects the Point.
     */
    updateDim(isAreaA, locArea, Location::INTERIOR, Dimension::P);
    /**
     * The area interior intersects the point's exterior neighbourhood.
     */
    updateDim(isAreaA, Location::INTERIOR, Location::EXTERIOR, Dimension::A);
    /**
     * If the area vertex is on the boundary,
     * the area boundary and exterior intersect the point's exterior neighbourhood
     */
    if (locArea == Location::BOUNDARY) {
        updateDim(isAreaA, Location::BOUNDARY, Location::EXTERIOR, Dimension::L);
        updateDim(isAreaA, Location::EXTERIOR, Location::EXTERIOR, Dimension::A);
    }
}


/* private */
void
TopologyComputer::addAreaVertexOnLine(bool isAreaA, Location locArea, Location locTarget, const CoordinateXY* pt)
{
    (void)pt;
    //-- Assert: locArea != EXTERIOR
    /**
     * If an area vertex intersects a line, all we know is the
     * intersection at that point.
     * e.g. the line may or may not be collinear with the area boundary,
     * and the line may or may not intersect the area interior.
     * Full topology is determined later by node analysis
     */
    updateDim(isAreaA, locArea, locTarget, Dimension::P);
    if (locArea == Location::INTERIOR) {
        /**
         * The area interior intersects the line's exterior neighbourhood.
         */
        updateDim(isAreaA, Location::INTERIOR, Location::EXTERIOR, Dimension::A);
    }
}


/* public */
void
TopologyComputer::addAreaVertexOnArea(bool isAreaA, Location locArea, Location locTarget, const CoordinateXY* pt)
{
    (void)pt;
    if (locTarget == Location::BOUNDARY) {
        if (locArea == Location::BOUNDARY) {
            //-- B/B topology is fully computed later by node analysis
            updateDim(isAreaA, Location::BOUNDARY, Location::BOUNDARY, Dimension::P);
        }
        else {
            // locArea == INTERIOR
            updateDim(isAreaA, Location::INTERIOR, Location::INTERIOR, Dimension::A);
            updateDim(isAreaA, Location::INTERIOR, Location::BOUNDARY, Dimension::L);
            updateDim(isAreaA, Location::INTERIOR, Location::EXTERIOR, Dimension::A);
        }
    }
    else {
        //-- locTarget is INTERIOR or EXTERIOR`
        updateDim(isAreaA, Location::INTERIOR, locTarget, Dimension::A);
        /**
         * If area vertex is on Boundary further topology can be deduced
         * from the neighbourhood around the boundary vertex.
         * This is always the case for polygonal geometries.
         * For GCs, the vertex may be either on boundary or in interior
         * (i.e. of overlapping or adjacent polygons)
         */
        if (locArea == Location::BOUNDARY) {
            updateDim(isAreaA, Location::BOUNDARY, locTarget, Dimension::L);
            updateDim(isAreaA, Location::EXTERIOR, locTarget, Dimension::A);
        }
    }
}


/* public */
void
TopologyComputer::evaluateNodes()
{
    for (auto& kv : nodeMap) {
        NodeSections* nodeSections = kv.second;
        if (nodeSections->hasInteractionAB()) {
            evaluateNode(nodeSections);
            if (isResultKnown())
                return;
        }
    }
}


/* private */
void
TopologyComputer::evaluateNode(NodeSections* nodeSections)
{
    const CoordinateXY* p = nodeSections->getCoordinate();
    std::unique_ptr<RelateNode> node = nodeSections->createNode();
    //-- Node must have edges for geom, but may also be in interior of a overlapping GC
    bool isAreaInteriorA = geomA.isNodeInArea(p, nodeSections->getPolygonal(RelateGeometry::GEOM_A));
    bool isAreaInteriorB = geomB.isNodeInArea(p, nodeSections->getPolygonal(RelateGeometry::GEOM_B));
    node->finish(isAreaInteriorA, isAreaInteriorB);
    evaluateNodeEdges(node.get());
}


/* private */
void
TopologyComputer::evaluateNodeEdges(const RelateNode* node)
{
    //TODO: collect distinct dim settings by using temporary matrix?
    for (const std::unique_ptr<RelateEdge>& e : node->getEdges()) {
        //-- An optimization to avoid updates for cases with a linear geometry
        if (isAreaArea()) {
            updateDim(e->location(RelateGeometry::GEOM_A, Position::LEFT),
                      e->location(RelateGeometry::GEOM_B, Position::LEFT), Dimension::A);
            updateDim(e->location(RelateGeometry::GEOM_A, Position::RIGHT),
                      e->location(RelateGeometry::GEOM_B, Position::RIGHT), Dimension::A);
        }
        updateDim(e->location(RelateGeometry::GEOM_A, Position::ON),
                  e->location(RelateGeometry::GEOM_B, Position::ON), Dimension::L);
    }
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


