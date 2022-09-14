/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/union/CoverageUnion.h>

#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/polygonize/Polygonizer.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/TopologyException.h>
#include <geos/shape/fractal/HilbertEncoder.h>

namespace geos {
namespace operation {
namespace geounion {

using geos::geom::Geometry;
using geos::geom::LineSegment;
using geos::geom::LineString;
using geos::geom::LinearRing;
using geos::geom::Polygon;
using geos::geom::GeometryCollection;
using geos::geom::GeometryFactory;
using geos::operation::polygonize::Polygonizer;

void CoverageUnion::extractRings(const Geometry* geom) {
    const Polygon* p = dynamic_cast<const Polygon*>(geom);
    if (p != nullptr) {
        extractRings(p);
    } else {
        auto gc = dynamic_cast<const GeometryCollection*>(geom);
        if (gc == nullptr) {
            throw geos::util::IllegalArgumentException("Unhandled geometry type in CoverageUnion.");
        }

        for (std::size_t i = 0; i < gc->getNumGeometries(); i++) {
            extractRings(gc->getGeometryN(i));
        }
    }
}

void CoverageUnion::extractRings(const Polygon* p) {
    const LinearRing* ring = p->getExteriorRing();

    rings.push_back(ring);
    for (std::size_t i = 0; i < p->getNumInteriorRing(); i++) {
        rings.push_back(p->getInteriorRingN(i));
    }
}

void CoverageUnion::sortRings() {
    shape::fractal::HilbertEncoder::sort(rings.begin(), rings.end());
}

void CoverageUnion::extractSegments(const LineString* ls) {
    auto coords = ls->getCoordinatesRO();

    if (coords->isEmpty())
        return;

    for (std::size_t i = 1; i < coords->size(); i++) {
        LineSegment segment{coords->getAt(i), coords->getAt(i-1)};
        segment.normalize();

       if (!segments.erase(segment)) {
           segments.emplace(std::move(segment));
       }
    }
}

std::unique_ptr<Geometry> CoverageUnion::polygonize(const GeometryFactory* gf) {
    Polygonizer p{true};

    // Create a vector to manage the lifecycle of a geometry corresponding to each line segment.
    // Polygonizer needs these to stay alive until it does its work.
    std::vector<std::unique_ptr<Geometry>> segment_geoms;
    segment_geoms.reserve(segments.size());

    for (const LineSegment& segment : segments) {
        auto seg_geom = segment.toGeometry(*gf);
        p.add(static_cast<Geometry*>(seg_geom.get()));
        segment_geoms.emplace_back(std::move(seg_geom));
    }

    if (!p.allInputsFormPolygons()) {
        throw geos::util::TopologyException("CoverageUnion cannot process incorrectly noded inputs.");
    }

    auto polygons = p.getPolygons();
    if (polygons.size() == 1) {
        return std::move(polygons[0]);
    }

    return gf->createMultiPolygon(std::move(polygons));
}

std::unique_ptr<geom::Geometry> CoverageUnion::Union(const geom::Geometry* geom) {
    CoverageUnion cu;

    cu.extractRings(geom);
    cu.sortRings();

    for (const auto& lr : cu.rings) {
        cu.extractSegments(lr);
    }

    double area_in = geom->getArea();

    auto ret = cu.polygonize(geom->getFactory());

    double area_out = ret->getArea();

    if (std::abs((area_out - area_in)/area_in) > AREA_PCT_DIFF_TOL) {
        throw geos::util::TopologyException("CoverageUnion cannot process overlapping inputs.");
    }

    return ret;
}

}
}
}
