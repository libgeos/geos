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

#pragma once

#include <geos/algorithm/NativeCurveSegments.h>
#include <geos/export.h>

#include <cstddef>
#include <string>

namespace geos {
namespace geom {
class Geometry;
}
}

namespace geos {
namespace io {

/**
 * Writes a geometry as Esri JSON, with curves kept as true curves.
 *
 * The tolerance must be set before writing (see setTolerance()).
 *
 * Points and multipoints are written as `x`/`y` and `points`; an empty
 * point as `{"x":null}`. Linear
 * geometries are written as `paths` and `rings`. A geometry with curved
 * parts is written as `curvePaths` or `curveRings`, whose segments are
 * lines (`[x, y]`), circular arcs (`{"c": [[x, y], [interiorX, interiorY]]}`),
 * elliptic arcs
 * (`{"a": [[x, y], [centerX, centerY], minor, clockwise, rotation, axis, ratio]}`)
 * and cubic Beziers (`{"b": [[x, y], [c1x, c1y], [c2x, c2y]]}`), the
 * form that ArcGIS reads with `arcpy.AsShape(json, True)`.
 *
 * Curve types are converted with algorithm::NativeCurveSegments: circular
 * strings, elliptical curves and the conic and cubic NURBS spans map
 * exactly, while clothoids, spiral curves and the other NURBS spans are
 * approximated by cubic Beziers that stay within the tolerance of the
 * source curve. See that class for the precise rules and for the meaning
 * of the tolerance.
 *
 * Z and M are not written. The SRID, when positive, is written as
 * `spatialReference.wkid`. Polygon exterior rings are written clockwise
 * and holes counterclockwise, as Esri JSON requires. A GeometryCollection
 * that is not a MultiCurve or MultiSurface is not supported and throws
 * util::UnsupportedOperationException.
 */
class GEOS_DLL EsriJsonWriter {

public:

    /// Counts about the last write(), for reporting what was approximated.
    struct Stats {
        /// Segments written, over all paths and rings.
        std::size_t numSegments = 0;
        /// Segments that approximate the source curve.
        std::size_t numApproximated = 0;
        /// Largest sampled deviation of any approximating segment.
        double maxDeviation = 0.0;
    };

    EsriJsonWriter() = default;

    /// The largest distance between an approximating Bezier and its
    /// source curve, in coordinate units. Must be positive and finite.
    /// There is no default, since a sensible value depends on the
    /// coordinate system: write() throws util::IllegalArgumentException
    /// until this is set.
    void setTolerance(double tolerance);

    double getTolerance() const
    {
        return params.tolerance;
    }

    /// Ask for curvature-matched (G2) Bezier fits where possible.
    /// Default true.
    void setMatchCurvature(bool match)
    {
        params.matchCurvature = match;
    }

    std::string write(const geom::Geometry& geometry);

    /// Statistics of the most recent write(); all zero when it threw.
    const Stats& getStats() const
    {
        return stats;
    }

private:

    algorithm::NativeCurveParams params;
    Stats stats;
};

}
}
