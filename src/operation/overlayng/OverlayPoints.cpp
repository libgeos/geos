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

using namespace geos::geom;

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

struct PointExtractingFilter final: public GeometryComponentFilter {

    PointExtractingFilter(std::map<CoordinateXY, std::unique_ptr<Point>>& p_ptMap, const PrecisionModel* p_pm)
        : ptMap(p_ptMap), pm(p_pm)
    {}

    void
    filter_ro(const Geometry* geom) override
    {
        if (geom->getGeometryTypeId() != GEOS_POINT) return;

        const Point* pt = static_cast<const Point*>(geom);
        // don't add empty points
        if (pt->isEmpty()) return;

        pt->getCoordinatesRO()->forEach([this, &pt](const auto& coord) -> void {
            auto rounded = roundCoord(coord, pm);

            /**
            * Only add first occurrence of a point.
            * This provides the merging semantics of overlay
            */
            if (ptMap.find(rounded) == ptMap.end()) {
                std::unique_ptr<Point> newPt(pt->getFactory()->createPoint(rounded));
                ptMap[rounded] = std::move(newPt);
            }
        });
    }

    template<typename CoordType>
    static CoordType
    roundCoord(const CoordType& p, const PrecisionModel* p_pm)
    {
        if (OverlayUtil::isFloating(p_pm))
            return p;
        CoordType p2(p);
        p_pm->makePrecise(p2);
        return p2;
    }

private:
    std::map<CoordinateXY, std::unique_ptr<Point>>& ptMap;
    const PrecisionModel* pm;
};

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
    PointMap map0 = buildPointMap(geom0);
    PointMap map1 = buildPointMap(geom1);

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
    if (rsltList.empty()) {
        uint8_t coordDim = OverlayUtil::resultCoordinateDimension(
                                            geom0->getCoordinateDimension(), 
                                            geom1->getCoordinateDimension());
        return OverlayUtil::createEmptyResult(0, coordDim, geometryFactory);
    }

    return geometryFactory->buildGeometry(std::move(rsltList));
}

/*private*/
void
OverlayPoints::computeIntersection(PointMap& map0,
                    PointMap& map1,
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
OverlayPoints::computeDifference(PointMap& map0,
                  PointMap& map1,
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
OverlayPoints::computeUnion(PointMap& map0,
             PointMap& map1,
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
std::map<CoordinateXY, std::unique_ptr<Point>>
OverlayPoints::buildPointMap(const Geometry* geom)
{
    std::map<CoordinateXY, std::unique_ptr<Point>> map;
    PointExtractingFilter filter(map, pm);
    geom->apply_ro(&filter);
    return map;
}

} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
