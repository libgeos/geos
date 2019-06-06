/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/prep/PreparedPolygonPredicate.java rev. 1.4 (JTS-1.10)
 * (2007-12-12)
 *
 **********************************************************************/

#include <geos/geom/prep/PreparedPolygonPredicate.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/util/ComponentCoordinateExtracter.h>
#include <geos/geom/Location.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
// std
#include <cstddef>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep
//
// private:
//

//
// protected:
//
struct AnyMatchingLocationFilter : public GeometryComponentFilter {
    explicit AnyMatchingLocationFilter(algorithm::locate::PointOnGeometryLocator* locator, int loc) :
        pt_locator(locator), test_loc(loc), found(false) {}

    algorithm::locate::PointOnGeometryLocator* pt_locator;
    const int test_loc;
    bool found;

    void filter_ro(const Geometry* g) override {
        auto pt = g->getCoordinate();
        if (!found) {
            const int loc = pt_locator->locate(pt);
            if (loc == test_loc) {
                found = true;
            }
        }
    }
};

struct AnyNotMatchingLocationFilter : public GeometryComponentFilter {
    explicit AnyNotMatchingLocationFilter(algorithm::locate::PointOnGeometryLocator* locator, int loc) :
            pt_locator(locator), test_loc(loc), found(false) {}

    algorithm::locate::PointOnGeometryLocator* pt_locator;
    const int test_loc;
    bool found;

    void filter_ro(const Geometry* g) override {
        auto pt = g->getCoordinate();
        if (!found) {
            const int loc = pt_locator->locate(pt);
            if (loc != test_loc) {
                found = true;
            }
        }
    }
};

bool
PreparedPolygonPredicate::isAllTestComponentsInTarget(const geom::Geometry* testGeom) const
{
    AnyMatchingLocationFilter filter(prepPoly->getPointLocator(), geom::Location::EXTERIOR);
    testGeom->apply_ro(&filter);

    return !filter.found;
}

bool
PreparedPolygonPredicate::isAllTestComponentsInTargetInterior(
    const geom::Geometry* testGeom) const
{
    AnyNotMatchingLocationFilter filter(prepPoly->getPointLocator(), geom::Location::INTERIOR);
    testGeom->apply_ro(&filter);

    return !filter.found;
}

bool
PreparedPolygonPredicate::isAnyTestComponentInTarget(
    const geom::Geometry* testGeom) const
{
    AnyNotMatchingLocationFilter filter(prepPoly->getPointLocator(), geom::Location::EXTERIOR);
    testGeom->apply_ro(&filter);

    return filter.found;
}

bool
PreparedPolygonPredicate::isAnyTestComponentInTargetInterior(
    const geom::Geometry* testGeom) const
{
    AnyMatchingLocationFilter filter(prepPoly->getPointLocator(), geom::Location::INTERIOR);
    testGeom->apply_ro(&filter);

    return filter.found;
}

bool
PreparedPolygonPredicate::isAnyTargetComponentInAreaTest(
    const geom::Geometry* testGeom,
    const geom::Coordinate::ConstVect* targetRepPts) const
{
    algorithm::locate::SimplePointInAreaLocator piaLoc(testGeom);

    for(std::size_t i = 0, ni = targetRepPts->size(); i < ni; i++) {
        const geom::Coordinate* pt = (*targetRepPts)[i];
        const int loc = piaLoc.locate(pt);
        if(geom::Location::EXTERIOR != loc) {
            return true;
        }
    }

    return false;
}

//
// public:
//

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos
