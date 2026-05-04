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

#include <geos/operation/split/SplitLinealAtPoint.h>

#include <geos/algorithm/CircularArcs.h>
#include <geos/algorithm/Interpolate.h>
#include <geos/geom/LineString.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/UnsupportedOperationException.h>

using geos::algorithm::CircularArcs;
using geos::geom::CircularArc;
using geos::geom::CircularString;
using geos::geom::CompoundCurve;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYZM;
using geos::geom::Curve;
using geos::geom::Geometry;
using geos::geom::GeometryTypeId;
using geos::geom::LineString;
using geos::geom::SimpleCurve;

namespace geos::operation::split {

std::pair<std::unique_ptr<LineString>, std::unique_ptr<LineString>>
SplitLinealAtPoint::splitLineStringAtPoint(const LineString& ls, std::size_t i, const CoordinateXY& pt)
{
    const auto& gf = *ls.getFactory();
    const CoordinateSequence& pts = *ls.getCoordinatesRO();

    if (pt.equals2D(pts.getAt<CoordinateXY>(i))) {
        return splitLineStringAtVertex(ls, i);
    }

    if (i + 1 >= pts.size()) {
        throw util::IllegalArgumentException("Cannot split LineString at point beyond end");
    }

    if (pt.equals2D(pts.getAt<CoordinateXY>(i + 1))) {
        return splitLineStringAtVertex(ls, i + 1);
    }

    auto pts1 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());
    auto pts2 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());

    CoordinateXYZM ptZM(pt);
    if (pts.hasZ() || pts.hasM()) {
        CoordinateXYZM prev;
        CoordinateXYZM next;
        pts.getAt(i, prev);
        pts.getAt(i + 1, next);

        ptZM.z = algorithm::Interpolate::zGetOrInterpolate(pt, prev, next);
        ptZM.m = algorithm::Interpolate::mGetOrInterpolate(pt, prev, next);
    }

    pts1->add(pts, 0, i);
    pts1->add(ptZM);

    if (i < pts.size() - 1) {
        pts2->add(ptZM);
        pts2->add(pts, i + 1, pts.size() - 1);
    }

    return { gf.createLineString(pts1), gf.createLineString(pts2) };
}

std::pair<std::unique_ptr<CircularString>, std::unique_ptr<CircularString>>
SplitLinealAtPoint::splitCircularStringAtPoint(const CircularString& cs, std::size_t i, const CoordinateXY& pt)
{
    const auto& gf = *cs.getFactory();
    const CoordinateSequence& pts = *cs.getCoordinatesRO();

    if (i % 2) {
        throw util::IllegalArgumentException("Section index must be the start of an arc");
    }

    if (pt.equals2D(pts.getAt<CoordinateXY>(i))) {
        return splitCircularStringAtVertex(cs, i);
    }

    if (i + 2 >= pts.size()) {
        throw util::IllegalArgumentException("Cannot split CircularString at arc beyond end");
    }

    const CircularArc arc(pts, i);

    if (pt.equals2D(arc.p2())) {
        return splitCircularStringAtVertex(cs, i + 2);
    }

    auto pts1 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());
    auto pts2 = std::make_shared<CoordinateSequence>(0, pts.hasZ(), pts.hasM());

    CoordinateXYZM splitPointZM(pt);
    if (pts.hasZ() || pts.hasM()) {
        CircularArcs::interpolateZM(pts, i, arc.getCenter(), arc.isCCW(), splitPointZM, splitPointZM.z, splitPointZM.m);
    }

    CoordinateXYZM sec0Midpoint(CircularArcs::getMidpoint(pts.getAt<CoordinateXY>(i), splitPointZM, arc.getCenter(), arc.getRadius(), arc.isCCW()));
    if (pts.hasZ() || pts.hasM()) {
        CircularArcs::interpolateZM(pts, i, arc.getCenter(), arc.isCCW(), sec0Midpoint, sec0Midpoint.z, sec0Midpoint.m);
    }
    pts1->add(pts, 0, i);
    pts1->add(sec0Midpoint);
    pts1->add(splitPointZM);

    CoordinateXYZM sec1Midpoint(CircularArcs::getMidpoint(splitPointZM, pts.getAt<CoordinateXY>(i + 2), arc.getCenter(), arc.getRadius(), arc.isCCW()));
    if (pts.hasZ() || pts.hasM()) {
        CircularArcs::interpolateZM(pts, i, arc.getCenter(), arc.isCCW(), sec1Midpoint, sec1Midpoint.z, sec1Midpoint.m);
    }
    pts2->add(splitPointZM);
    pts2->add(sec1Midpoint);
    pts2->add(pts, i + 2, pts.size() - 1);

    return { gf.createCircularString(pts1), gf.createCircularString(pts2) };
}

std::pair<std::unique_ptr<LineString>, std::unique_ptr<LineString>>
SplitLinealAtPoint::splitLineStringAtVertex(const LineString& ls, std::size_t i)
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
        SplitLinealAtPoint::splitCircularStringAtVertex(const CircularString& cs, std::size_t i)
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
        SplitLinealAtPoint::splitSimpleCurveAtVertex(const SimpleCurve& sc, std::size_t i)
{
    if (sc.getGeometryTypeId() == GeometryTypeId::GEOS_CIRCULARSTRING) {
        return splitCircularStringAtVertex(static_cast<const CircularString&>(sc), i);
    }
    if (sc.getGeometryTypeId() == GeometryTypeId::GEOS_LINESTRING || sc.getGeometryTypeId() == GeometryTypeId::GEOS_LINEARRING) {
        return splitLineStringAtVertex(static_cast<const LineString&>(sc), i);
    }

    throw util::UnsupportedOperationException("Unhandled type in SplitLinealAtPoint::splitAtVertex");
}

std::pair<std::unique_ptr<SimpleCurve>, std::unique_ptr<SimpleCurve>>
SplitLinealAtPoint::splitSimpleCurveAtPoint(const SimpleCurve& sc, std::size_t i, const CoordinateXY& pt)
{
    if (sc.getGeometryTypeId() == GeometryTypeId::GEOS_CIRCULARSTRING) {
        return splitCircularStringAtPoint(static_cast<const CircularString&>(sc), i, pt);
    }
    if (sc.getGeometryTypeId() == GeometryTypeId::GEOS_LINESTRING || sc.getGeometryTypeId() == GeometryTypeId::GEOS_LINEARRING) {
        return splitLineStringAtPoint(static_cast<const LineString&>(sc), i, pt);
    }

    throw util::UnsupportedOperationException("Unhandled type in SplitLinealAtPoint::splitAtVertex");
}

template<typename T>
void removeEmptyGeometries(T& geoms) {
    geoms.erase(std::remove_if(geoms.begin(), geoms.end(), [](const auto& curve) { return curve->isEmpty(); }), geoms.end());
}

static std::unique_ptr<Curve>
makeCurve(const geom::GeometryFactory& gfact, std::vector<std::unique_ptr<SimpleCurve>> curves)
{
    if (curves.size() == 1) {
        return std::move(curves[0]);
    }

    removeEmptyGeometries(curves);

    if (curves.size() == 1) {
        return std::move(curves[0]);
    }

    return gfact.createCompoundCurve(std::move(curves));
}

std::pair<std::unique_ptr<Curve>, std::unique_ptr<Curve>>
SplitLinealAtPoint::splitCompoundCurveAtPoint(const CompoundCurve& cc, std::size_t component, std::size_t section, const CoordinateXY& pt)
{
    const auto& gfact = *cc.getFactory();

    std::vector<std::unique_ptr<SimpleCurve>> firstCurves;
    std::vector<std::unique_ptr<SimpleCurve>> secondCurves;

    for (std::size_t i = 0; i < cc.getNumCurves(); i++) {
        const auto* sc = cc.getCurveN(i);

        if (i < component) {
            firstCurves.push_back(sc->clone());
        } else if (i == component) {
            auto split = splitSimpleCurveAtPoint(*sc, section, pt);
            firstCurves.push_back(std::move(split.first));
            secondCurves.push_back(std::move(split.second));
        } else {
            secondCurves.push_back(sc->clone());
        }
    }

    std::pair<std::unique_ptr<Curve>, std::unique_ptr<Curve>> result;
    result.first = makeCurve(gfact, std::move(firstCurves));
    result.second = makeCurve(gfact, std::move(secondCurves));

    return result;
}

}