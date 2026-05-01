/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/split/GeometrySplitter.h>

#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/util/GeometryCombiner.h>
#include <geos/geom/util/GeometryTransformer.h>
#include <geos/geom/util/LinealExtracter.h>
#include <geos/geom/util/LinearComponentExtracter.h>
#include <geos/geom/util/PointExtracter.h>
#include <geos/geom/util/PolygonalExtracter.h>
#include <geos/noding/GeometryNoder.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/operation/distance/GeometryLocation.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/operation/split/SplitLinealAtPoint.h>
#include <geos/shape/random/RandomPointsBuilder.h>
#include <geos/util/Assert.h>

using geos::geom::CircularString;
using geos::geom::CoordinateXY;
using geos::geom::CompoundCurve;
using geos::geom::Curve;
using geos::geom::CurvePolygon;
using geos::geom::Geometry;
using geos::geom::GeometryCollection;
using geos::geom::Point;
using geos::geom::LineString;
using geos::geom::MultiLineString;
using geos::geom::MultiPoint;
using geos::geom::Polygon;
using geos::geom::SimpleCurve;
using geos::geom::prep::PreparedGeometryFactory;
using geos::geom::util::GeometryCombiner;
using geos::geom::util::GeometryTransformer;
using geos::geom::util::LinealExtracter;
using geos::geom::util::PointExtracter;
using geos::geom::util::PolygonalExtracter;
using geos::noding::GeometryNoder;
using geos::operation::polygonize::Polygonizer;
using geos::operation::distance::DistanceOp;
using geos::operation::distance::GeometryLocation;
using geos::shape::random::RandomPointsBuilder;

namespace geos::operation::split {

class GeometrySplitter::SplitWithPointTransformer : public GeometryTransformer {
public:
    explicit SplitWithPointTransformer(const Point& pt) : m_pt(pt) {}

protected:
    std::unique_ptr<Geometry>
    transformCircularString(const CircularString* geom, const Geometry* /*parent*/) override
    {
        return splitCurveWithPoint(*geom, m_pt);
    }

    std::unique_ptr<Geometry>
    transformCompoundCurve(const CompoundCurve* geom, const Geometry* /*parent*/) override
    {
        return splitCurveWithPoint(*geom, m_pt);
    }

    std::unique_ptr<Geometry>
    transformLineString(const LineString* geom, const Geometry* /*parent*/) override
    {
        return splitCurveWithPoint(*geom, m_pt);
    }

    std::unique_ptr<Geometry>
    transformMultiLineString(const MultiLineString* mls, const Geometry* parent) override
    {
        return transformGeometryCollection(mls, parent);
    }

    std::unique_ptr<Geometry>
    transformPolygon(const Polygon*, const Geometry*) override
    {
        throw geos::util::IllegalArgumentException("Splitting a Polygon with a point is not supported.");
    }

    std::unique_ptr<Geometry>
    transformCurvePolygon(const CurvePolygon*, const Geometry*) override
    {
        throw geos::util::IllegalArgumentException("Splitting a CurvePolygon with a point is not supported.");
    }

    std::unique_ptr<Geometry>
    transformGeometryCollection(const GeometryCollection* inputGC, const Geometry* /*parent*/) override
    {
        std::vector<std::unique_ptr<Geometry>> geoms;

        for (const auto& input : *inputGC) {
            auto transformed = transform(input.get());
            GeometryCollection* transformedGC = detail::down_cast<GeometryCollection*>(transformed.get());

            for (auto& g : transformedGC->releaseGeometries()) {
                if (!g->isEmpty()) {
                    geoms.push_back(std::move(g));
                }
            }
        }

        return inputGC->getFactory()->createGeometryCollection(std::move(geoms));
    }

private:
    const Point& m_pt;
};

class RemoveCoordinateZM : public geom::CoordinateSequenceFilter {

public:

    void filter_rw(geom::CoordinateSequence& seq, std::size_t) override {
        seq.setZM(false, false);
        m_done = true;
    }

    bool isDone() const override {
        return m_done;
    }

    bool isGeometryChanged() const override {
        // We didn't change the XY coords; no need to update the envelope.
        return false;
    }

private:
    bool m_done{false};
};

std::unique_ptr<Geometry>
GeometrySplitter::splitAtPoints(const Geometry& geom, const Geometry& splitPoints)
{
    const Geometry* toSplit = &geom;

    std::vector<const Point*> points;
    PointExtracter::getPoints(splitPoints, points);

    std::unique_ptr<Geometry> result;

    for (const auto& point : points) {
        SplitWithPointTransformer xform(*point);
        result = xform.transform(toSplit);
        toSplit = result.get();
    }

    return result;
}

std::unique_ptr<Geometry>
GeometrySplitter::split(const Geometry &geom, const Geometry &splitGeom)
{
    if (geom.isEmpty() || splitGeom.isEmpty()) {
        std::vector<std::unique_ptr<Geometry>> geoms;
        geoms.push_back(geom.clone());
        return geom.getFactory()->createGeometryCollection(std::move(geoms));
    }

    if (splitGeom.hasZ() || splitGeom.hasM()) {
        RemoveCoordinateZM xform;
        auto splitGeom2D = splitGeom.clone();
        splitGeom2D->apply_rw(xform);
        assert(!splitGeom2D->hasZ());
        assert(!splitGeom2D->hasM());
        return split(geom, *splitGeom2D);
    }

    const Geometry* toSplit = &geom;

    std::unique_ptr<Geometry> splitByPointsResult;
    if (splitGeom.hasDimension(geom::Dimension::P)) {
        splitByPointsResult = splitAtPoints(geom, splitGeom);

        if (splitGeom.isDimensionStrict(geom::Dimension::P)) {
            return splitByPointsResult;
        }

        toSplit = splitByPointsResult.get();
    }

    // If our input has mixed areas and lines, split them separately, then recombine.
    if (toSplit->hasDimension(geom::Dimension::A) && toSplit->hasDimension(geom::Dimension::L)) {
        std::vector<const Geometry*> lines;
        LinealExtracter::getLineals(toSplit, lines);
        auto linearInput = geom.getFactory()->createGeometryCollection(lines);
        auto linearOutput = splitLinealWithEdge(*linearInput, splitGeom);

        std::vector<const Geometry*> polys;
        PolygonalExtracter::getPolygonals(*toSplit, polys);
        auto polygonalInput = geom.getFactory()->createGeometryCollection(polys);
        auto polygonalOutput = splitPolygonalWithEdge(*polygonalInput, splitGeom);

        return GeometryCombiner::combine(std::move(linearOutput), std::move(polygonalOutput));
    }

    if (toSplit->hasDimension(geom::Dimension::A)) {
        return splitPolygonalWithEdge(*toSplit, splitGeom);
    }

    return splitLinealWithEdge(*toSplit, splitGeom);
}


std::unique_ptr<Geometry>
GeometrySplitter::splitCurveWithPoint(const Curve& g, const Point& point)
{
    if (g.isEmpty()) {
        std::vector<std::unique_ptr<Geometry>> geoms;
        geoms.push_back(g.clone());
        return g.getFactory()->createGeometryCollection(std::move(geoms));
    }

    DistanceOp distance(g, point);

    if (distance.distance() > POINT_TO_LINE_TOLERANCE) {
        std::vector<std::unique_ptr<Geometry>> geoms;
        geoms.push_back(g.clone());
        return g.getFactory()->createGeometryCollection(std::move(geoms));
    }

    const auto& nearestLoc = distance.nearestLocations()[0];

    std::pair<std::unique_ptr<Curve>, std::unique_ptr<Curve>> split;
    if (g.getGeometryTypeId() == geom::GEOS_COMPOUNDCURVE) {
        const CompoundCurve& cc = static_cast<const CompoundCurve&>(g);

        for (std::size_t i = 0 ; i < cc.getNumCurves(); i++) {
            const auto* sc = cc.getCurveN(i);

            if (sc == nearestLoc.getGeometryComponent()) {
                split = SplitLinealAtPoint::splitCompoundCurveAtPoint(cc, i, nearestLoc.getSegmentIndex(), nearestLoc.getCoordinate());
                break;
            }
        }
    } else {
        const SimpleCurve& sc = static_cast<const SimpleCurve&>(g);
        split = SplitLinealAtPoint::splitSimpleCurveAtPoint(sc, nearestLoc.getSegmentIndex(), nearestLoc.getCoordinate());
    }

    util::Assert::isNotNull(split.first, "Split result first curve should not be null");
    util::Assert::isNotNull(split.second, "Split result second curve should not be null");

    std::vector<std::unique_ptr<Geometry>> geoms;
    if (!split.first->isEmpty()) {
        geoms.push_back(std::move(split.first));
    }
    if (!split.second->isEmpty()) {
        geoms.push_back(std::move(split.second));
    }

    return g.getFactory()->createGeometryCollection(std::move(geoms));
}

std::unique_ptr<Geometry>
GeometrySplitter::splitLinealWithEdge(const Geometry &geom, const Geometry &edge)
{
    if (!geom.isDimensionStrict(geom::Dimension::L)) {
        throw util::IllegalArgumentException("Input geometry must be linear or polygonal.");
    }

    if (geom.isEmpty()) {
        std::vector<std::unique_ptr<Geometry>> geoms;
        geoms.push_back(geom.clone());
        return geom.getFactory()->createGeometryCollection(std::move(geoms));
    }

    GeometryNoder noder(geom, edge);
    noder.setOnlyFirstGeomEdges(true);

    auto nodedMLS = noder.getNoded();

    auto nodedGC = geom.getFactory()->createGeometryCollection(detail::down_cast<GeometryCollection*>(nodedMLS.get())->releaseGeometries());

    return nodedGC;
}

static std::unique_ptr<Point>
getInteriorPoint(const geom::Surface& surface)
{
    if (!surface.hasCurvedComponents()) {
        return surface.getInteriorPoint();
    }

    RandomPointsBuilder rpb(surface.getFactory());
    rpb.setNumPoints(1);
    rpb.setExtent(surface);
    auto mp = rpb.getGeometry();

    auto geoms = detail::down_cast<MultiPoint*>(mp.get())->releaseGeometries();

    return std::unique_ptr<Point>(detail::down_cast<Point*>(geoms[0].release()));
}

std::unique_ptr<Geometry>
GeometrySplitter::splitPolygonalWithEdge(const Geometry &geom, const Geometry &edge)
{
    if (!geom.isDimensionStrict(geom::Dimension::A)) {
        throw util::IllegalArgumentException("Input geometry must be polygonal.");
    }

    auto noded = GeometryNoder::node(geom, edge);

    Polygonizer polygonizer;
    polygonizer.add(noded.get());

    auto surfaces = polygonizer.getSurfaces();

    const bool usePrepared = !geom.hasCurvedComponents() && !edge.hasCurvedComponents();
    std::vector<std::unique_ptr<Geometry>> keep;

    if (usePrepared) {
        auto prepGeom = PreparedGeometryFactory::prepare(&geom);

        for (auto& surface : surfaces)
        {
            const auto testPoint = surface->getInteriorPoint();
            if (prepGeom->intersects(testPoint.get())) {
                keep.push_back(std::move(surface));
            }
        }
    } else {
        for (auto& surface : surfaces)
        {
            const auto testPoint = getInteriorPoint(*surface);
            if (geom.intersects(testPoint.get())) {
                keep.push_back(std::move(surface));
            }
        }
    }

    if (keep.empty()) {
        return geom.getFactory()->createEmptyGeometry(geom::GEOS_GEOMETRYCOLLECTION, geom.hasZ(), geom.hasM());
    }

    return geom.getFactory()->createGeometryCollection(std::move(keep));
}

}
