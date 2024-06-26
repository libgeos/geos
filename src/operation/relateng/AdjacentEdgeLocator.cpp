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


#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/relateng/AdjacentEdgeLocator.h>
#include <geos/operation/relateng/NodeSection.h>
#include <geos/operation/relateng/NodeSections.h>
#include <geos/operation/relateng/RelateNode.h>
#include <geos/constants.h>


using geos::algorithm::Orientation;
using geos::algorithm::PointLocation;
using geos::geom::CoordinateXY;
using geos::geom::Dimension;
using geos::geom::Geometry;
using geos::geom::LinearRing;
using geos::geom::Location;
using geos::geom::Polygon;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public */
Location
AdjacentEdgeLocator::locate(const CoordinateXY* p)
{
    NodeSections sections(p);
    for (const CoordinateSequence* ring : ringList) {
        addSections(p, ring, sections);
    }
    std::unique_ptr<RelateNode> node = sections.createNode();

    return node->hasExteriorEdge(true) ? Location::BOUNDARY : Location::INTERIOR;
}


/* private */
void
AdjacentEdgeLocator::addSections(
    const CoordinateXY* p,
    const CoordinateSequence* ring,
    NodeSections& sections)
{
    for (std::size_t i = 0; i < ring->getSize() - 1; i++) {
        const CoordinateXY& p0 = ring->getAt(i);
        const CoordinateXY& pnext = ring->getAt(i+1);
        if (p->equals2D(pnext)) {
            //-- segment final point is assigned to next segment
            continue;
        }
        else if (p->equals2D(p0)) {
            std::size_t iprev = i > 0 ? i - 1 : ring->getSize() - 2;
            const CoordinateXY& pprev = ring->getAt(iprev);
            NodeSection *ns = createSection(p, &pprev, &pnext);
            sections.addNodeSection(ns);
        }
        else if (PointLocation::isOnSegment(*p, p0, pnext)) {
            NodeSection *ns = createSection(p, &p0, &pnext);
            sections.addNodeSection(ns);
        }
    }
}


/* private */
NodeSection*
AdjacentEdgeLocator::createSection(const CoordinateXY* p,
    const CoordinateXY* prev,
    const CoordinateXY* next)
{
    if (prev->distance(*p) == 0 || next->distance(*p) == 0) {
        //System.out.println("Found zero-length section segment");
    };
    return new NodeSection(true, Dimension::A, 1, 0, nullptr, false, prev, p, next);
}


/* private */
void
AdjacentEdgeLocator::init(const Geometry* geom)
{
    if (geom->isEmpty())
        return;
    addRings(geom);
}


/* private */
void
AdjacentEdgeLocator::addRings(const Geometry* geom)
{
    if (const Polygon* poly = dynamic_cast<const Polygon*>(geom)) {
        const LinearRing* shell = poly->getExteriorRing();
        addRing(shell, true);
        for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
            const LinearRing* hole = poly->getInteriorRingN(i);
            addRing(hole, false);
        }
    }
    else if (geom->isCollection()) {
        //-- recurse through collections
        for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
            addRings(geom->getGeometryN(i));
        }
    }
}


/* private */
void
AdjacentEdgeLocator::addRing(const LinearRing* ring, bool requireCW)
{
    //TODO: remove repeated points?
    const CoordinateSequence* pts = ring->getCoordinatesRO();
    bool isFlipped = requireCW == Orientation::isCCW(pts);
    /*
     * In case of flipped rings, we need to keep a local copy
     * since we cannot mutate the const geometry we are fed
     * in the constructor.
     */
    if (isFlipped) {
        std::unique_ptr<CoordinateSequence> localPts = pts->clone();
        localPts->reverse();
        ringList.push_back(localPts.get());
        localRingList.push_back(std::move(localPts));
    }
    else {
        ringList.push_back(pts);
    }
    return;
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
