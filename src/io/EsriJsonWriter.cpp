/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Jeroen Bloemscheer
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/io/EsriJsonWriter.h>

#include <geos/io/CLocalizer.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Point.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <locale>
#include <sstream>

using geos::algorithm::NativeCurvePath;
using geos::algorithm::NativeCurveSegment;
using geos::algorithm::NativeCurveSegments;
using geos::geom::CoordinateXY;

namespace geos {
namespace io {

namespace {

void
writeNumber(std::ostream& os, double v)
{
    if (!std::isfinite(v)) {
        throw util::IllegalArgumentException("Esri JSON cannot represent a non-finite ordinate");
    }
    // The shortest of 15, 16 or 17 significant digits that reads back as v.
    char buf[40];
    for (int digits = 15; digits <= 17; digits++) {
        std::snprintf(buf, sizeof buf, "%.*g", digits, v);
        if (digits == 17 || std::strtod(buf, nullptr) == v) {
            break;
        }
    }
    os << buf;
}

void
writePoint(std::ostream& os, const CoordinateXY& p)
{
    os << '[';
    writeNumber(os, p.x);
    os << ',';
    writeNumber(os, p.y);
    os << ']';
}

void
writeSegment(std::ostream& os, const NativeCurveSegment& s)
{
    using Type = NativeCurveSegment::Type;
    switch (s.type) {
    case Type::LINE:
        writePoint(os, s.end);
        return;
    case Type::CIRCULAR_ARC:
        os << "{\"c\":[";
        writePoint(os, s.end);
        os << ',';
        writePoint(os, s.interior);
        os << "]}";
        return;
    case Type::ELLIPTIC_ARC:
        os << "{\"a\":[";
        writePoint(os, s.end);
        os << ',';
        writePoint(os, s.center);
        os << ',' << (s.minor ? 1 : 0) << ',' << (s.clockwise ? 1 : 0) << ',';
        writeNumber(os, s.rotation);
        os << ',';
        writeNumber(os, s.semiMajorAxis);
        os << ',';
        writeNumber(os, s.axisRatio);
        os << "]}";
        return;
    case Type::CUBIC_BEZIER:
        os << "{\"b\":[";
        writePoint(os, s.end);
        os << ',';
        writePoint(os, s.control1);
        os << ',';
        writePoint(os, s.control2);
        os << "]}";
        return;
    }
}

void
writePath(std::ostream& os, const NativeCurvePath& path)
{
    os << '[';
    if (!path.segments.empty()) {
        writePoint(os, path.segments.front().start);
        for (const NativeCurveSegment& s : path.segments) {
            os << ',';
            writeSegment(os, s);
        }
    }
    os << ']';
}

void
writePlainPath(std::ostream& os, const NativeCurvePath& path)
{
    os << '[';
    if (!path.segments.empty()) {
        writePoint(os, path.segments.front().start);
        for (const NativeCurveSegment& s : path.segments) {
            os << ',';
            writePoint(os, s.end);
        }
    }
    os << ']';
}

bool
isRingType(geom::GeometryTypeId id)
{
    return id == geom::GEOS_POLYGON || id == geom::GEOS_CURVEPOLYGON
           || id == geom::GEOS_MULTIPOLYGON || id == geom::GEOS_MULTISURFACE;
}

void
writePoints(std::ostream& os, const geom::Geometry& g)
{
    if (g.getGeometryTypeId() == geom::GEOS_POINT) {
        const auto& pt = static_cast<const geom::Point&>(g);
        if (pt.isEmpty()) {
            os << "{\"x\":null";
        }
        else {
            os << "{\"x\":";
            writeNumber(os, pt.getX());
            os << ",\"y\":";
            writeNumber(os, pt.getY());
        }
        return;
    }
    os << "{\"points\":[";
    bool first = true;
    for (std::size_t i = 0; i < g.getNumGeometries(); i++) {
        const auto& pt = static_cast<const geom::Point&>(*g.getGeometryN(i));
        if (pt.isEmpty()) {
            continue;
        }
        if (!first) {
            os << ',';
        }
        first = false;
        writePoint(os, CoordinateXY(pt.getX(), pt.getY()));
    }
    os << ']';
}

} // anonymous namespace

void
EsriJsonWriter::setTolerance(double tolerance)
{
    if (!(tolerance > 0.0) || !std::isfinite(tolerance)) {
        throw util::IllegalArgumentException("Esri JSON curve tolerance must be positive and finite");
    }
    params.tolerance = tolerance;
}

std::string
EsriJsonWriter::write(const geom::Geometry& g)
{
    CLocalizer clocale;
    stats = Stats();
    if (!(params.tolerance > 0.0) || !std::isfinite(params.tolerance)) {
        throw util::IllegalArgumentException(
            "Esri JSON curve tolerance must be set to a positive, finite value");
    }
    std::ostringstream os;
    os.imbue(std::locale::classic());

    const geom::GeometryTypeId id = g.getGeometryTypeId();
    if (id == geom::GEOS_POINT || id == geom::GEOS_MULTIPOINT) {
        writePoints(os, g);
    }
    else {
        const std::vector<NativeCurvePath> paths = NativeCurveSegments::convert(g, params);
        stats.numSegments = NativeCurveSegments::countSegments(paths);
        stats.numApproximated = NativeCurveSegments::countApproximations(paths);
        stats.maxDeviation = NativeCurveSegments::maxDeviation(paths);

        bool curved = false;
        for (const NativeCurvePath& p : paths) {
            for (const NativeCurveSegment& s : p.segments) {
                if (s.type != NativeCurveSegment::Type::LINE) {
                    curved = true;
                }
            }
        }
        const bool rings = isRingType(id);
        const char* key = rings ? (curved ? "curveRings" : "rings")
                          : (curved ? "curvePaths" : "paths");
        os << "{\"" << key << "\":[";
        for (std::size_t i = 0; i < paths.size(); i++) {
            if (i > 0) {
                os << ',';
            }
            if (curved) {
                writePath(os, paths[i]);
            }
            else {
                writePlainPath(os, paths[i]);
            }
        }
        os << ']';
    }
    if (g.getSRID() > 0) {
        os << ",\"spatialReference\":{\"wkid\":" << g.getSRID() << '}';
    }
    os << '}';
    return os.str();
}

}
}
