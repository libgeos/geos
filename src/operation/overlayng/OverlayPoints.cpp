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

#include <geos/operation/overlayng/OverlayPoints.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/OverlayUtil.h>



namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng


/*public static*/
std::unique_ptr<Geometry>
OverlayPoints::overlay(int opCode, const Geometry* geom0, const Geometry* geom1, const PrecisionModel* pm)
{
    OverlayPoints overlay(opCode, geom0, geom1, pm);
    return overlay.getResult();
}


/*public*/
std::unique_ptr<Geometry>
OverlayPoints::getResult()
{
    std::map<Coordinate, std::unique_ptr<Point>> map0 = buildPointMap(geom0);
    std::map<Coordinate, std::unique_ptr<Point>> map1 = buildPointMap(geom1);

    std::vector<std::unique_ptr<Point>> rsltList;
    switch (opCode) {
        case OverlayNG::INTERSECTION: {
            computeIntersection(map0, map1, rsltList);
            break;
        }
        case OverlayNG::UNION: {
            computeUnion(map0, map1, rsltList);
            break;
        }
        case OverlayNG::DIFFERENCE: {
            computeDifference(map0, map1, rsltList);
            break;
        }
        case OverlayNG::SYMDIFFERENCE: {
            computeDifference(map0, map1, rsltList);
            computeDifference(map1, map0, rsltList);
            break;
        }
    }
    if (rsltList.empty())
        return OverlayUtil::createEmptyResult(0, geometryFactory);

    return geometryFactory->buildGeometry(std::move(rsltList));
}

/*private*/
void
OverlayPoints::computeIntersection(std::map<Coordinate, std::unique_ptr<Point>>& map0,
                    std::map<Coordinate, std::unique_ptr<Point>>& map1,
                    std::vector<std::unique_ptr<Point>>& rsltList)
{
    // for each entry in map0
    for (auto& ent : map0) {
        // is there an entry in map1?
        const auto& it = map1.find(ent.first);
        if (it != map1.end()) {
            // add it to the result, taking ownership
            rsltList.emplace_back(ent.second.release());
        }
    }
}

/*private*/
void
OverlayPoints::computeDifference(std::map<Coordinate, std::unique_ptr<Point>>& map0,
                  std::map<Coordinate, std::unique_ptr<Point>>& map1,
                  std::vector<std::unique_ptr<Point>>& rsltList)
{
    // for each entry in map0
    for (auto& ent : map0) {
        // is there no entry in map1?
        const auto& it = map1.find(ent.first);
        if (it == map1.end()) {
            // add it to the result, taking ownership
            rsltList.emplace_back(ent.second.release());
        }
    }
}

/*private*/
void
OverlayPoints::computeUnion(std::map<Coordinate, std::unique_ptr<Point>>& map0,
             std::map<Coordinate, std::unique_ptr<Point>>& map1,
             std::vector<std::unique_ptr<Point>>& rsltList)
{
    // take all map0 points
    for (auto& ent : map0) {
        rsltList.emplace_back(ent.second.release());
    }

    // find any map1 points that aren't already in the result
    for (auto& ent : map1) {
        // is there no entry in map0?
        const auto& it = map0.find(ent.first);
        if (it == map0.end()) {
            // add it to the result, taking ownership
            rsltList.emplace_back(ent.second.release());
        }
    }
}


/*private*/
std::map<Coordinate, std::unique_ptr<Point>>
OverlayPoints::buildPointMap(const Geometry* geom)
{
    std::map<Coordinate, std::unique_ptr<Point>> map;
    for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
        const Geometry* elt = geom->getGeometryN(i);
        if (elt->getGeometryTypeId() != GEOS_POINT) {
            throw util::IllegalArgumentException("Non-point geometry input to point overlay");
        }
        // don't add empty points
        if (elt->isEmpty()) continue;

        const Point* pt = static_cast<const Point*>(elt);
        Coordinate p = roundCoord(pt, pm);
        /**
        * Only add first occurrence of a point.
        * This provides the merging semantics of overlay
        */
        if (map.find(p) == map.end()) {
            std::unique_ptr<Point> newPt(pt->getFactory()->createPoint(p));
            map[p] = std::move(newPt);
        }
    }
    return map;
}


/*private*/
Coordinate
OverlayPoints::roundCoord(const Point* pt, const PrecisionModel* p_pm) const
{
    const Coordinate* p = pt->getCoordinate();
    if (OverlayUtil::isFloating(p_pm))
        return *p;
    Coordinate p2 = *p;
    p_pm->makePrecise(p2);
    return p2;
}








} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
