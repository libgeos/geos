/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/split/SplitGeometryAtVertex.h>

#include <geos/geom/LineString.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/UnsupportedOperationException.h>

using geos::geom::CoordinateSequence;
using geos::geom::CircularString;
using geos::geom::LineString;
using geos::geom::SimpleCurve;
using geos::geom::GeometryTypeId;

namespace geos::operation::split {
std::pair<std::unique_ptr<LineString>, std::unique_ptr<LineString>>
        SplitGeometryAtVertex::splitLineStringAtVertex(const LineString& ls, std::size_t i)
{
    const auto& gf = *ls.getFactory();
    const CoordinateSequence& pts = *ls.getCoordinatesRO();

    auto pts1 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());
    auto pts2 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());

    if (i > 0) {
        pts1->add(pts, 0, i);
    }
    if (i < pts.size() - 1) {
        pts2->add(pts, i, pts.size() - 1);
    }

    return { gf.createLineString(pts1), gf.createLineString(pts2) };
}

std::pair<std::unique_ptr<CircularString>, std::unique_ptr<CircularString>>
        SplitGeometryAtVertex::splitCircularStringAtVertex(const CircularString& cs, std::size_t i)
{
    const auto& gf = *cs.getFactory();
    const CoordinateSequence& pts = *cs.getCoordinatesRO();

    if (i % 2) {
        throw util::IllegalArgumentException("Cannot split CircularString at arc control point");
    }

    auto pts1 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());
    auto pts2 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());

    if (i > 0) {
        pts1->add(pts, 0, i);
    }
    if (i < pts.size() - 1) {
        pts2->add(pts, i, pts.size() - 1);
    }

    return { gf.createCircularString(pts1), gf.createCircularString(pts2) };
}

std::pair<std::unique_ptr<SimpleCurve>, std::unique_ptr<SimpleCurve>>
        SplitGeometryAtVertex::splitSimpleCurveAtVertex(const SimpleCurve& sc, std::size_t i)
{
    if (sc.getGeometryTypeId() == GeometryTypeId::GEOS_CIRCULARSTRING) {
        return splitCircularStringAtVertex(static_cast<const CircularString&>(sc), i);
    }
    if (sc.getGeometryTypeId() == GeometryTypeId::GEOS_LINESTRING || sc.getGeometryTypeId() == GeometryTypeId::GEOS_LINEARRING) {
        return splitLineStringAtVertex(static_cast<const LineString&>(sc), i);
    }

    throw util::UnsupportedOperationException("Unhandled type in SplitGeometryAtVertex::splitAtVertex");
}

}