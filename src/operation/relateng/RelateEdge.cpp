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
#include <geos/operation/relateng/RelateEdge.h>
#include <geos/operation/relateng/RelateNode.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Position.h>
#include <geos/io/WKTWriter.h>
#include <geos/constants.h>
#include <sstream>

using geos::algorithm::PolygonNodeTopology;
using geos::geom::CoordinateXY;
using geos::geom::Dimension;
using geos::geom::Location;
using geos::geom::Position;
using geos::io::WKTWriter;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public */
RelateEdge::RelateEdge(const RelateNode* rNode, const CoordinateXY* pt, bool isA, bool isForward)
    : node(rNode)
    , dirPt(pt)
{
    setLocationsArea(isA, isForward);
}


/* public */
RelateEdge::RelateEdge(const RelateNode* rNode, const CoordinateXY* pt, bool isA)
    : node(rNode)
    , dirPt(pt)
{
    setLocationsLine(isA);
}


/* public */
RelateEdge::RelateEdge(const RelateNode* rNode, const CoordinateXY* pt,
    bool isA, Location locLeft, Location locRight, Location locLine)
    : node(rNode)
    , dirPt(pt)
{
    setLocations(isA, locLeft, locRight, locLine);
}


/* public static */
RelateEdge *
RelateEdge::create(const RelateNode* node, const CoordinateXY* dirPt, bool isA, int dim, bool isForward)
{
    if (dim == Dimension::A)
        //-- create an area edge
        return new RelateEdge(node, dirPt, isA, isForward);
    //-- create line edge
    return new RelateEdge(node, dirPt, isA);
}


/* public static */
std::size_t
RelateEdge::findKnownEdgeIndex(
    std::vector<std::unique_ptr<RelateEdge>>& edges,
    bool isA)
{
    for (std::size_t i = 0; i < edges.size(); i++) {
        auto& e = edges[i];
        if (e->isKnown(isA))
            return i;
    }
    return INDEX_UNKNOWN;
}


/* public static */
void
RelateEdge::setAreaInterior(
    std::vector<std::unique_ptr<RelateEdge>>& edges,
    bool isA)
{
    for (auto& e : edges) {
        e->setAreaInterior(isA);
    }
}


/* private */
void
RelateEdge::setLocations(bool isA, Location locLeft, Location locRight, Location locLine)
{
    if (isA) {
        aDim = 2;
        aLocLeft = locLeft;
        aLocRight = locRight;
        aLocLine = locLine;
    }
    else {
        bDim = 2;
        bLocLeft = locLeft;
        bLocRight = locRight;
        bLocLine = locLine;
    }
}


/* private */
void
RelateEdge::setLocationsLine(bool isA)
{
    if (isA) {
        aDim = 1;
        aLocLeft = Location::EXTERIOR;
        aLocRight = Location::EXTERIOR;
        aLocLine = Location::INTERIOR;
    }
    else {
        bDim = 1;
        bLocLeft = Location::EXTERIOR;
        bLocRight = Location::EXTERIOR;
        bLocLine = Location::INTERIOR;
    }
}


/* private */
void
RelateEdge::setLocationsArea(bool isA, bool isForward)
{
    Location locLeft = isForward ? Location::EXTERIOR : Location::INTERIOR;
    Location locRight = isForward ? Location::INTERIOR : Location::EXTERIOR;
    if (isA) {
        aDim = 2;
        aLocLeft = locLeft;
        aLocRight = locRight;
        aLocLine = Location::BOUNDARY;
    }
    else {
        bDim = 2;
        bLocLeft = locLeft;
        bLocRight = locRight;
        bLocLine = Location::BOUNDARY;
    }
}


/* public */
int
RelateEdge::compareToEdge(const CoordinateXY* edgeDirPt) const
{
    return PolygonNodeTopology::compareAngle(
        node->getCoordinate(),
        dirPt,
        edgeDirPt);
}


/* public */
void
RelateEdge::merge(bool isA, int dim, bool isForward)
{
    Location locEdge = Location::INTERIOR;
    Location locLeft = Location::EXTERIOR;
    Location locRight = Location::EXTERIOR;
    if (dim == Dimension::A) {
        locEdge = Location::BOUNDARY;
        locLeft = isForward ? Location::EXTERIOR : Location::INTERIOR;
        locRight = isForward ? Location::INTERIOR : Location::EXTERIOR;
    }

    if (! isKnown(isA)) {
        setDimension(isA, dim);
        setOn(isA, locEdge);
        setLeft(isA, locLeft);
        setRight(isA, locRight);
        return;
    }

    // Assert: node-dirpt is collinear with node-pt
    mergeDimEdgeLoc(isA, locEdge);
    mergeSideLocation(isA, Position::LEFT, locLeft);
    mergeSideLocation(isA, Position::RIGHT, locRight);
}

/**
* Area edges override Line edges.
* Merging edges of same dimension is a no-op for
* the dimension and on location.
* But merging an area edge into a line edge
* sets the dimension to A and the location to BOUNDARY.
*
* @param isA
* @param locEdge
*/
/* private */
void
RelateEdge::mergeDimEdgeLoc(bool isA, Location locEdge)
{
    //TODO: this logic needs work - ie handling A edges marked as Interior
    int dim = (locEdge == Location::BOUNDARY) ? Dimension::A : Dimension::L;
    if (dim == Dimension::A && dimension(isA) == Dimension::L) {
        setDimension(isA, dim);
        setOn(isA, Location::BOUNDARY);
    }
}


/* private */
void
RelateEdge::mergeSideLocation(bool isA, int pos, Location loc)
{
    Location currLoc = location(isA, pos);
    //-- INTERIOR takes precedence over EXTERIOR
    if (currLoc != Location::INTERIOR) {
        setLocation(isA, pos, loc);
    }
}


/* private */
void
RelateEdge::setDimension(bool isA, int dimension)
{
    if (isA) {
        aDim = dimension;
    }
    else {
        bDim = dimension;
    }
}


/* public */
void
RelateEdge::setLocation(bool isA, int pos, Location loc)
{
    switch (pos) {
    case Position::LEFT:
        setLeft(isA, loc);
        break;
    case Position::RIGHT:
        setRight(isA, loc);
        break;
    case Position::ON:
        setOn(isA, loc);
        break;
    }
}


/* public */
void
RelateEdge::setAllLocations(bool isA, Location loc)
{
    setLeft(isA, loc);
    setRight(isA, loc);
    setOn(isA, loc);
}


/* public */
void
RelateEdge::setUnknownLocations(bool isA, Location loc)
{
    if (! isKnown(isA, Position::LEFT)) {
        setLocation(isA, Position::LEFT, loc);
    }
    if (! isKnown(isA, Position::RIGHT)) {
        setLocation(isA, Position::RIGHT, loc);
    }
    if (! isKnown(isA, Position::ON)) {
        setLocation(isA, Position::ON, loc);
    }
}


/* private */
void
RelateEdge::setLeft(bool isA, Location loc)
{
    if (isA) {
        aLocLeft = loc;
    }
    else {
        bLocLeft = loc;
    }
}


/* private */
void
RelateEdge::setRight(bool isA, Location loc)
{
    if (isA) {
        aLocRight = loc;
    }
    else {
        bLocRight = loc;
    }
}


/* private */
void
RelateEdge::setOn(bool isA, Location loc)
{
    if (isA) {
        aLocLine = loc;
    }
    else {
        bLocLine = loc;
    }
}


/* public */
Location
RelateEdge::location(bool isA, int position) const
{
    if (isA) {
        switch (position) {
            case Position::LEFT: return aLocLeft;
            case Position::RIGHT: return aLocRight;
            case Position::ON: return aLocLine;
        }
    }
    else {
        switch (position) {
            case Position::LEFT: return bLocLeft;
            case Position::RIGHT: return bLocRight;
            case Position::ON: return bLocLine;
        }
    }
    assert(false && "never get here");
    return LOC_UNKNOWN;
}

/* private */
int
RelateEdge::dimension(bool isA) const
{
    return isA ? aDim : bDim;
}

/* private */
bool
RelateEdge::isKnown(bool isA) const
{
    if (isA)
        return aDim != DIM_UNKNOWN;
    return bDim != DIM_UNKNOWN;
}

/* private */
bool
RelateEdge::isKnown(bool isA, int pos) const
{
    return location(isA, pos) != LOC_UNKNOWN;
}


/* public */
bool
RelateEdge::isInterior(bool isA, int position) const
{
    return location(isA, position) == Location::INTERIOR;
}


/* public */
void
RelateEdge::setDimLocations(bool isA, int dim, Location loc)
{
    if (isA) {
        aDim = dim;
        aLocLeft = loc;
        aLocRight = loc;
        aLocLine = loc;
    }
    else {
        bDim = dim;
        bLocLeft = loc;
        bLocRight = loc;
        bLocLine = loc;
    }
}


/* public */
void
RelateEdge::setAreaInterior(bool isA)
{
    if (isA) {
        aLocLeft = Location::INTERIOR;
        aLocRight = Location::INTERIOR;
        aLocLine = Location::INTERIOR;
    }
    else {
        bLocLeft = Location::INTERIOR;
        bLocRight = Location::INTERIOR;
        bLocLine = Location::INTERIOR;
    }
}


/* public */
std::string
RelateEdge::toString() const
{
    std::stringstream ss;
    ss << WKTWriter::toLineString(*(node->getCoordinate()), *dirPt);
    ss << " - " << labelString();
    return ss.str();
}


/* private */
std::string
RelateEdge::labelString() const
{
    std::stringstream ss;
    ss << "A:";
    ss << locationString(RelateGeometry::GEOM_A);
    ss << "/B:";
    ss << locationString(RelateGeometry::GEOM_B);
    return ss.str();
}


/* private */
std::string
RelateEdge::locationString(bool isA) const
{
    std::stringstream ss;
    ss << location(isA, Position::LEFT);
    ss << location(isA, Position::ON);
    ss << location(isA, Position::RIGHT);
    return ss.str();
}


/* public friend */
std::ostream&
operator<<(std::ostream& os, const RelateEdge& re)
{
    os << re.toString();
    return os;
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos




