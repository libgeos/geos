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

#include <geos/export.h>
#include <geos/geom/Coordinate.h>

#include <cstddef>
#include <limits>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Curve;
class Geometry;
}
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace algorithm {

/**
 * One segment of a curve expressed in the segment kinds that desktop GIS
 * geometry engines (for instance ArcGIS) represent natively: a straight
 * line, a circular arc, an elliptic arc or a cubic Bezier.
 *
 * Only X and Y are carried; Z and M are dropped.
 *
 * The segment starts where the previous segment of its path ended, which
 * is stored in `start` as well so a segment can be read on its own.
 *
 * \c exact tells whether the segment IS the source curve (up to double
 * rounding) or an approximation of it. An approximation is a cubic Bezier
 * fitted to the source curve; `maxDeviation` is then the largest distance
 * between the Bezier and the source curve found by dense sampling (see
 * NativeCurveParams). It is a measurement, not a proof, and for an exact
 * segment it is 0.
 */
struct GEOS_DLL NativeCurveSegment {

    enum class Type {
        LINE,
        CIRCULAR_ARC,
        ELLIPTIC_ARC,
        CUBIC_BEZIER
    };

    Type type = Type::LINE;

    /// Start point.
    geom::CoordinateXY start;

    /// End point.
    geom::CoordinateXY end;

    /// CIRCULAR_ARC: a point on the arc strictly between start and end.
    geom::CoordinateXY interior;

    /// CIRCULAR_ARC and ELLIPTIC_ARC: the centre.
    geom::CoordinateXY center;

    /// CIRCULAR_ARC: the radius.
    double radius = 0.0;

    /// ELLIPTIC_ARC: the semi-major axis length.
    double semiMajorAxis = 0.0;

    /// ELLIPTIC_ARC: semi-minor axis length divided by semi-major axis
    /// length, in (0, 1].
    double axisRatio = 1.0;

    /// ELLIPTIC_ARC: the angle of the major axis in radians,
    /// counterclockwise from +x.
    double rotation = 0.0;

    /// CIRCULAR_ARC and ELLIPTIC_ARC: true when the arc runs clockwise
    /// from start to end.
    bool clockwise = false;

    /// ELLIPTIC_ARC: true when the arc spans less than half the ellipse.
    bool minor = true;

    /// CUBIC_BEZIER: the first and second control points.
    geom::CoordinateXY control1;
    geom::CoordinateXY control2;

    /// False when this segment approximates the source curve.
    bool exact = true;

    /// For an approximation, the largest sampled distance between this
    /// segment and the source curve. 0 for an exact segment.
    double maxDeviation = 0.0;

    /// The same segment traced from end to start.
    NativeCurveSegment reversed() const;
};

/// A connected run of segments. The path starts at the start of its first
/// segment. `closed` is set when it came from a ring, whose last end is
/// then its start.
struct GEOS_DLL NativeCurvePath {
    std::vector<NativeCurveSegment> segments;
    bool closed = false;
};

/**
 * Parameters of the conversion.
 *
 * `tolerance` bounds the distance between the source curve and every
 * approximating cubic Bezier. It is in the units of the coordinates, so
 * the right value depends on the coordinate system (0.001 is a millimetre
 * in metres but about 111 m in degrees), and for that reason it has NO
 * default: it starts as NaN and a conversion with it unset throws
 * util::IllegalArgumentException. It has no effect on segments that are
 * exact.
 *
 * The bound is measured, not proven. The measure is a sampled Hausdorff
 * distance: n + 1 points of the source curve and of the Bezier, each
 * projected onto the other curve near where the previous point matched. It
 * is taken with at least 256 points per segment, and a segment that
 * measures above `tolerance` is split and refitted. A curve whose features
 * are finer than the sampling can in principle hide a larger deviation.
 *
 * `matchCurvature` asks for G2 fits, that is fits that also match the
 * curvature of the source at both ends of each Bezier, when that is
 * possible; otherwise the fit is only G1 (positions and tangents).
 */
struct GEOS_DLL NativeCurveParams {
    double tolerance = std::numeric_limits<double>::quiet_NaN();
    bool matchCurvature = true;
};

/**
 * Converts curve geometries into NativeCurveSegment paths.
 *
 * Exact mappings (`exact` is true):
 *  - LineString, LinearRing: lines;
 *  - CircularString: circular arcs (a collinear arc becomes a line, a
 *    complete circle becomes two arcs);
 *  - EllipticalCurve: elliptic arcs, or circular arcs when the placed
 *    ellipse is a circle. An arc longer than 120 degrees of parameter is
 *    split into equal pieces, so each piece is a minor arc;
 *  - NurbsCurve: every Bezier span of the curve is classified by its
 *    algebra. A span of degree 1 becomes a line. A non-rational span of
 *    degree up to 3, or of higher degree that is exactly a cubic, becomes
 *    a cubic Bezier (a quadratic is degree elevated exactly). A rational
 *    quadratic span whose weights make it an ellipse or circle becomes an
 *    elliptic or circular arc, and one that makes it a parabola becomes a
 *    cubic Bezier.
 *  - CompoundCurve, CurvePolygon rings, MultiCurve: the sections above.
 *
 * Approximations (`exact` is false), each a chain of cubic Beziers that
 * is G1 continuous and, when NativeCurveParams::matchCurvature is set and
 * the solve converges, G2 matched at the Bezier ends:
 *  - Clothoid and SpiralCurve (all five spiral types);
 *  - NurbsCurve spans that are rational and not an ellipse, circle or
 *    parabola (hyperbolas, degree above 2), and polynomial spans of
 *    degree above 3 that are not exactly a cubic.
 *
 * Throws util::IllegalArgumentException for a tolerance that is not
 * positive and finite, and util::UnsupportedOperationException for a
 * geometry type that has no path form (a GeometryCollection that is not a
 * MultiCurve or MultiSurface) or a clothoid with a singular placement.
 */
class GEOS_DLL NativeCurveSegments {

public:

    /// The paths of a LineString, LinearRing or any curve type, one path.
    static NativeCurvePath convertCurve(const geom::Curve& curve,
                                        const NativeCurveParams& params);

    /// The paths of a curve, MultiCurve, MultiLineString, or the rings of
    /// a Polygon, CurvePolygon, MultiPolygon or MultiSurface (exterior
    /// rings first, as they occur). Rings have `closed` set. Points have
    /// no path.
    static std::vector<NativeCurvePath> convert(const geom::Geometry& geom,
            const NativeCurveParams& params);

    /// Number of segments that are approximations, over all paths.
    static std::size_t countApproximations(const std::vector<NativeCurvePath>& paths);

    /// Total number of segments over all paths.
    static std::size_t countSegments(const std::vector<NativeCurvePath>& paths);

    /// Largest maxDeviation over all segments.
    static double maxDeviation(const std::vector<NativeCurvePath>& paths);
};

}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
