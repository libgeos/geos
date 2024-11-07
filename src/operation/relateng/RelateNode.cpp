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

#include <geos/operation/relateng/RelateNode.h>
#include <geos/operation/relateng/RelateEdge.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <geos/operation/relateng/NodeSection.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Position.h>
#include <geos/geom/Coordinate.h>
#include <geos/io/WKTWriter.h>
#include <geos/constants.h>


using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::Dimension;
using geos::geom::Position;
using geos::io::WKTWriter;
using geos::geom::Location;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public */
const CoordinateXY*
RelateNode::getCoordinate() const
{
    return nodePt;
}


/* public */
const std::vector<std::unique_ptr<RelateEdge>>&
RelateNode::getEdges() const
{
    return edges;
}


/* public */
void
RelateNode::addEdges(std::vector<const NodeSection *>& nss)
{
    for (auto* ns : nss) {
        addEdges(ns);
    }
}

/* public */
void
RelateNode::addEdges(std::vector<std::unique_ptr<NodeSection>>& nss)
{
    for (auto& ns : nss) {
        addEdges(ns.get());
    }
}

/* private */
std::size_t
RelateNode::indexOf(
    const std::vector<std::unique_ptr<RelateEdge>>& vEdges,
    const RelateEdge* edge) const
{
    for (std::size_t i = 0; i < vEdges.size(); i++)
    {
        const std::unique_ptr<RelateEdge>& e = vEdges[i];
        if (e.get() == edge)
            return i;
    }
    return INDEX_UNKNOWN;
}


/* public */
void
RelateNode::addEdges(const NodeSection* ns)
{
  //Debug.println("Adding NS: " + ns);
    switch (ns->dimension()) {
    case Dimension::L:
        addLineEdge(ns->isA(), ns->getVertex(0));
        addLineEdge(ns->isA(), ns->getVertex(1));
        break;
    case Dimension::A:
        //-- assumes node edges have CW orientation (as per JTS norm)
        //-- entering edge - interior on L
        const RelateEdge* e0 = addAreaEdge(ns->isA(), ns->getVertex(0), false);
        //-- exiting edge - interior on R
        const RelateEdge* e1 = addAreaEdge(ns->isA(), ns->getVertex(1), true);

        std::size_t index0 = indexOf(edges, e0);
        std::size_t index1 = indexOf(edges, e1);
        updateEdgesInArea(ns->isA(), index0, index1);
        updateIfAreaPrev(ns->isA(), index0);
        updateIfAreaNext(ns->isA(), index1);
    }
}


/* private */
void
RelateNode::updateEdgesInArea(bool isA, std::size_t indexFrom, std::size_t indexTo)
{
    std::size_t index = nextIndex(edges, indexFrom);
    while (index != indexTo) {
        auto& edge = edges[index];
        edge->setAreaInterior(isA);
        index = nextIndex(edges, index);
    }
}


/* private */
void
RelateNode::updateIfAreaPrev(bool isA, std::size_t index)
{
    std::size_t indexPrev = prevIndex(edges, index);
    auto& edgePrev = edges[indexPrev];
    if (edgePrev->isInterior(isA, Position::LEFT)) {
        std::unique_ptr<RelateEdge>& edge = edges[index];
        edge->setAreaInterior(isA);
    }
}


/* private */
void
RelateNode::updateIfAreaNext(bool isA, std::size_t index)
{
    std::size_t indexNext = nextIndex(edges, index);
    auto& edgeNext = edges[indexNext];
    if (edgeNext->isInterior(isA, Position::RIGHT)) {
        auto& edge = edges[index];
        edge->setAreaInterior(isA);
    }
}


/* private */
const RelateEdge*
RelateNode::addLineEdge(bool isA, const CoordinateXY* dirPt)
{
    return addEdge(isA, dirPt, Dimension::L, false);
}


/* private */
const RelateEdge*
RelateNode::addAreaEdge(bool isA, const CoordinateXY* dirPt, bool isForward)
{
    return addEdge(isA, dirPt, Dimension::A, isForward);
}


/* private */
const RelateEdge*
RelateNode::addEdge(bool isA, const CoordinateXY* dirPt, int dim, bool isForward)
{
    //-- check for well-formed edge - skip null or zero-len input
    if (dirPt == nullptr)
        return nullptr;
    if (nodePt->equals2D(*dirPt))
        return nullptr;

    std::size_t insertIndex = INDEX_UNKNOWN;
    for (std::size_t i = 0; i < edges.size(); i++) {
        auto* e = edges[i].get();
        int comp = e->compareToEdge(dirPt);
        if (comp == 0) {
            e->merge(isA, dim, isForward);
            return e;
        }
        if (comp == 1) {
            //-- found further edge, so insert a new edge at this position
            insertIndex = i;
            break;
        }
    }
    //-- add a new edge
    RelateEdge* e = RelateEdge::create(this, dirPt, isA, dim, isForward);
    if (insertIndex == INDEX_UNKNOWN) {
        //-- add edge at end of list
        edges.emplace_back(e);
    }
    else {
        //-- add edge before higher edge found
        std::unique_ptr<RelateEdge> re(e);
        edges.insert(
            edges.begin() + static_cast<long>(insertIndex),
            std::move(re));
    }
    return e;
}


/* public */
void
RelateNode::finish(bool isAreaInteriorA, bool isAreaInteriorB)
{
    //Debug.println("finish Node.");
    //Debug.println("Before: " + this);
    finishNode(RelateGeometry::GEOM_A, isAreaInteriorA);
    finishNode(RelateGeometry::GEOM_B, isAreaInteriorB);
    //Debug.println("After: " + this);
}


/* private */
void
RelateNode::finishNode(bool isA, bool isAreaInterior)
{
    if (isAreaInterior) {
        RelateEdge::setAreaInterior(edges, isA);
    }
    else {
        std::size_t startIndex = RelateEdge::findKnownEdgeIndex(edges, isA);
        //-- only interacting nodes are finished, so this should never happen
        //Assert.isTrue(startIndex >= 0l, "Node at "+ nodePt + "does not have AB interaction");
        propagateSideLocations(isA, startIndex);
    }
}


/* private */
void
RelateNode::propagateSideLocations(bool isA, std::size_t startIndex)
{
    Location currLoc = edges[startIndex]->location(isA, Position::LEFT);
    //-- edges are stored in CCW order
    std::size_t index = nextIndex(edges, startIndex);
    while (index != startIndex) {
        const auto& e = edges[index];
        e->setUnknownLocations(isA, currLoc);
        currLoc = e->location(isA, Position::LEFT);
        index = nextIndex(edges, index);
    }
}


/* private static */
std::size_t
RelateNode::prevIndex(
    std::vector<std::unique_ptr<RelateEdge>>& list,
    std::size_t index)
{
    if (index > 0 && index != INDEX_UNKNOWN) {
        return index - 1;
    }
    //-- index == 0
    return list.size() - 1;
}


/* private static */
std::size_t
RelateNode::nextIndex(
    std::vector<std::unique_ptr<RelateEdge>>& list,
    std::size_t index)
{
    if (index >= list.size() - 1 || index == INDEX_UNKNOWN) {
        return 0;
    }
    return index + 1;
}


/* public */
bool
RelateNode::hasExteriorEdge(bool isA)
{
    for (auto& e : edges) {
        if (Location::EXTERIOR == e->location(isA, Position::LEFT) ||
            Location::EXTERIOR == e->location(isA, Position::RIGHT))
        {
            return true;
        }
    }
    return false;
}


/* public */
std::string
RelateNode::toString() const
{
    std::stringstream ss;
    ss << "Node[" << WKTWriter::toPoint(*nodePt) << "]:" << std::endl;
    for (auto& e : edges) {
        ss << e->toString() << std::endl;
    }
    return ss.str();
}


/* public friend */
std::ostream&
operator<<(std::ostream& os, const RelateNode& rn)
{
    os << rn.toString();
    return os;
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


