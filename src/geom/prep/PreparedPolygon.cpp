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
 * Last port: geom/prep/PreparedPolygon.java rev 1.7 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/geom/Polygon.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/geom/prep/PreparedPolygonContains.h>
#include <geos/geom/prep/PreparedPolygonContainsProperly.h>
#include <geos/geom/prep/PreparedPolygonCovers.h>
#include <geos/geom/prep/PreparedPolygonDistance.h>
#include <geos/geom/prep/PreparedPolygonIntersects.h>
#include <geos/geom/prep/PreparedPolygonPredicate.h>
#include <geos/noding/FastSegmentSetIntersectionFinder.h>
#include <geos/noding/SegmentStringUtil.h>
#include <geos/operation/predicate/RectangleContains.h>
#include <geos/operation/predicate/RectangleIntersects.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
// std
#include <cstddef>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep
//
// public:
//
PreparedPolygon::PreparedPolygon(const geom::Geometry* geom)
    : BasicPreparedGeometry(geom)
{
    isRectangle = getGeometry().isRectangle();
}

PreparedPolygon::~PreparedPolygon()
{
    for(std::size_t i = 0, ni = segStrings.size(); i < ni; i++) {
        delete segStrings[ i ];
    }
}


noding::FastSegmentSetIntersectionFinder*
PreparedPolygon::
getIntersectionFinder() const
{
    if(! segIntFinder) {
        noding::SegmentStringUtil::extractSegmentStrings(&getGeometry(), segStrings);
        segIntFinder.reset(new noding::FastSegmentSetIntersectionFinder(&segStrings));
    }
    return segIntFinder.get();
}

algorithm::locate::PointOnGeometryLocator*
PreparedPolygon::
getPointLocator() const
{
    // If we are only going to locate a single point, it's faster to do a brute-force SimplePointInAreaLocator
    // instead of an IndexedPointInAreaLocator. There's a reasonable chance we will only use this locator
    // once (for example, if we get here through Geometry::intersects). So we create a simple locator for the
    // first usage and switch to an indexed locator when it is clear we're in a multiple-use scenario.
    if(! ptOnGeomLoc) {
        ptOnGeomLoc = detail::make_unique<algorithm::locate::SimplePointInAreaLocator>(&getGeometry());
        return ptOnGeomLoc.get();
    } else if (!indexedPtOnGeomLoc) {
        indexedPtOnGeomLoc = detail::make_unique<algorithm::locate::IndexedPointInAreaLocator>(getGeometry());
    }

    return indexedPtOnGeomLoc.get();
}

bool
PreparedPolygon::
contains(const geom::Geometry* g) const
{
    // short-circuit test
    if(!envelopeCovers(g)) {
        return false;
    }

    // optimization for rectangles
    if(isRectangle) {
        geom::Geometry const& geom = getGeometry();
        geom::Polygon const& poly = *detail::down_cast<geom::Polygon const*>(&geom);

        return operation::predicate::RectangleContains::contains(poly, *g);
    }

    return PreparedPolygonContains::contains(this, g);
}

bool
PreparedPolygon::
containsProperly(const geom::Geometry* g) const
{
    // short-circuit test
    if(!envelopeCovers(g)) {
        return false;
    }

    return PreparedPolygonContainsProperly::containsProperly(this, g);
}

bool
PreparedPolygon::
covers(const geom::Geometry* g) const
{
    // short-circuit test
    if(!envelopeCovers(g)) {
        return false;
    }

    // optimization for rectangle arguments
    if(isRectangle) {
        return true;
    }

    return PreparedPolygonCovers::covers(this, g);
}

bool
PreparedPolygon::
intersects(const geom::Geometry* g) const
{
    // envelope test
    if(!envelopesIntersect(g)) {
        return false;
    }

    // optimization for rectangles
    if(isRectangle) {
        geom::Geometry const& geom = getGeometry();
        geom::Polygon const& poly = dynamic_cast<geom::Polygon const&>(geom);

        return operation::predicate::RectangleIntersects::intersects(poly, *g);
    }

    return PreparedPolygonIntersects::intersects(this, g);
}

/* public */
operation::distance::IndexedFacetDistance*
PreparedPolygon::
getIndexedFacetDistance() const
{
    if(! indexedDistance ) {
        indexedDistance.reset(new operation::distance::IndexedFacetDistance(&getGeometry()));
    }
    return indexedDistance.get();
}

double
PreparedPolygon::distance(const geom::Geometry* g) const
{
    return PreparedPolygonDistance::distance(*this, g);
}

bool
PreparedPolygon::isWithinDistance(const geom::Geometry* g, double d) const
{
    return PreparedPolygonDistance(*this).isWithinDistance(g, d);
}

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos
