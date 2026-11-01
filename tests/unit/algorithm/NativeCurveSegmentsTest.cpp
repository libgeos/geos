//
// Test Suite for geos::algorithm::NativeCurveSegments
//
// The exact mappings are checked by decoding every segment back into points
// the way a consumer of Esri JSON would (circle through three points, ellipse
// from centre, axis, ratio, rotation and direction, Bezier by de Casteljau)
// and comparing with the source curve. The approximations are checked against
// a dense reference of the source curve that does not use the converter.

#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/algorithm/CurveToLineParams.h>
#include <geos/algorithm/NativeCurveSegments.h>
#include <geos/constants.h>
#include <geos/geom/Clothoid.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/EllipticalCurve.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/NurbsCurve.h>
#include <geos/geom/SpiralCurve.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using geos::algorithm::CurveToLineParams;
using geos::algorithm::NativeCurveParams;
using geos::algorithm::NativeCurvePath;
using geos::algorithm::NativeCurveSegment;
using geos::algorithm::NativeCurveSegments;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::NurbsCurve;
using geos::geom::NurbsKnot;
using geos::geom::NurbsPoint;
using Type = NativeCurveSegment::Type;

namespace {

const double PI = geos::MATH_PI;

double
cross(const CoordinateXY& a, const CoordinateXY& b)
{
    return a.x * b.y - a.y * b.x;
}

CoordinateXY
sub(const CoordinateXY& a, const CoordinateXY& b)
{
    return CoordinateXY(a.x - b.x, a.y - b.y);
}

double
len(const CoordinateXY& a)
{
    return std::hypot(a.x, a.y);
}

double
fold(double a)
{
    double r = std::fmod(a, 2 * PI);
    return r < 0 ? r + 2 * PI : r;
}

// --- decoding of segments, from the definition of each segment kind -------

struct Circle {
    CoordinateXY center;
    double radius;
};

Circle
circumcircle(const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c)
{
    const double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
    const double a2 = a.x * a.x + a.y * a.y;
    const double b2 = b.x * b.x + b.y * b.y;
    const double c2 = c.x * c.x + c.y * c.y;
    Circle out;
    out.center = CoordinateXY((a2 * (b.y - c.y) + b2 * (c.y - a.y) + c2 * (a.y - b.y)) / d,
                              (a2 * (c.x - b.x) + b2 * (a.x - c.x) + c2 * (b.x - a.x)) / d);
    out.radius = len(sub(a, out.center));
    return out;
}

// Points along one segment, including both ends.
std::vector<CoordinateXY>
decode(const NativeCurveSegment& s, std::size_t n)
{
    std::vector<CoordinateXY> pts;
    switch (s.type) {
    case Type::LINE:
        for (std::size_t i = 0; i <= n; i++) {
            const double t = static_cast<double>(i) / static_cast<double>(n);
            pts.push_back(CoordinateXY(s.start.x + t * (s.end.x - s.start.x),
                                       s.start.y + t * (s.end.y - s.start.y)));
        }
        break;
    case Type::CUBIC_BEZIER:
        for (std::size_t i = 0; i <= n; i++) {
            const double t = static_cast<double>(i) / static_cast<double>(n);
            CoordinateXY p[4] = {s.start, s.control1, s.control2, s.end};
            for (int r = 1; r < 4; r++) {
                for (int k = 0; k < 4 - r; k++) {
                    p[k] = CoordinateXY((1 - t) * p[k].x + t * p[k + 1].x,
                                        (1 - t) * p[k].y + t * p[k + 1].y);
                }
            }
            pts.push_back(p[0]);
        }
        break;
    case Type::CIRCULAR_ARC: {
        // Esri: end point and an interior point; the start is implicit.
        const Circle c = circumcircle(s.start, s.interior, s.end);
        const double a0 = std::atan2(s.start.y - c.center.y, s.start.x - c.center.x);
        const double a1 = std::atan2(s.interior.y - c.center.y, s.interior.x - c.center.x);
        const double a2 = std::atan2(s.end.y - c.center.y, s.end.x - c.center.x);
        const bool ccw = cross(sub(s.interior, s.start), sub(s.end, s.interior)) > 0;
        const double sweep = ccw ? fold(a1 - a0) + fold(a2 - a1) : -(fold(a0 - a1) + fold(a1 - a2));
        for (std::size_t i = 0; i <= n; i++) {
            const double t = a0 + sweep * static_cast<double>(i) / static_cast<double>(n);
            pts.push_back(CoordinateXY(c.center.x + c.radius * std::cos(t),
                                       c.center.y + c.radius * std::sin(t)));
        }
        break;
    }
    case Type::ELLIPTIC_ARC: {
        const double a = s.semiMajorAxis;
        const double b = s.semiMajorAxis * s.axisRatio;
        const double cr = std::cos(s.rotation);
        const double sr = std::sin(s.rotation);
        auto param = [&](const CoordinateXY& p) {
            const CoordinateXY r = sub(p, s.center);
            return std::atan2((-r.x * sr + r.y * cr) / b, (r.x * cr + r.y * sr) / a);
        };
        const double p0 = param(s.start);
        const double p1 = param(s.end);
        double sweep = s.clockwise ? -fold(p0 - p1) : fold(p1 - p0);
        // Esri's minor flag: the arc is less than half the ellipse.
        if (s.minor && std::abs(sweep) > PI) {
            sweep = sweep > 0 ? sweep - 2 * PI : sweep + 2 * PI;
        }
        if (!s.minor && std::abs(sweep) < PI) {
            sweep = sweep > 0 ? sweep - 2 * PI : sweep + 2 * PI;
        }
        for (std::size_t i = 0; i <= n; i++) {
            const double t = p0 + sweep * static_cast<double>(i) / static_cast<double>(n);
            const double x = a * std::cos(t);
            const double y = b * std::sin(t);
            pts.push_back(CoordinateXY(s.center.x + x * cr - y * sr, s.center.y + x * sr + y * cr));
        }
        break;
    }
    }
    return pts;
}

double
pointSegmentDistance(const CoordinateXY& p, const CoordinateXY& a, const CoordinateXY& b)
{
    const CoordinateXY ab = sub(b, a);
    const double l2 = ab.x * ab.x + ab.y * ab.y;
    double t = l2 > 0 ? ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / l2 : 0.0;
    t = std::min(1.0, std::max(0.0, t));
    return len(sub(p, CoordinateXY(a.x + t * ab.x, a.y + t * ab.y)));
}

std::vector<CoordinateXY>
decodePath(const NativeCurvePath& path, std::size_t perSegment)
{
    std::vector<CoordinateXY> pts;
    for (const NativeCurveSegment& s : path.segments) {
        const std::vector<CoordinateXY> d = decode(s, perSegment);
        pts.insert(pts.end(), d.begin(), d.end());
    }
    return pts;
}

// A polyline with a uniform grid over its segments, for nearest queries.
class PolylineIndex {
public:
    explicit PolylineIndex(const std::vector<CoordinateXY>& line)
        : m_line(line)
    {
        double minX = line[0].x, maxX = line[0].x, minY = line[0].y, maxY = line[0].y;
        for (const CoordinateXY& p : line) {
            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
        }
        m_minX = minX;
        m_minY = minY;
        const double extent = std::max(std::max(maxX - minX, maxY - minY), 1e-12);
        m_cell = extent / static_cast<double>(N - 1);
        m_cells.resize(N * N);
        for (std::size_t i = 0; i + 1 < line.size(); i++) {
            const std::size_t x0 = cellOf(std::min(line[i].x, line[i + 1].x) - m_minX);
            const std::size_t x1 = cellOf(std::max(line[i].x, line[i + 1].x) - m_minX);
            const std::size_t y0 = cellOf(std::min(line[i].y, line[i + 1].y) - m_minY);
            const std::size_t y1 = cellOf(std::max(line[i].y, line[i + 1].y) - m_minY);
            for (std::size_t x = x0; x <= x1; x++) {
                for (std::size_t y = y0; y <= y1; y++) {
                    m_cells[y * N + x].push_back(i);
                }
            }
        }
    }

    double distance(const CoordinateXY& p) const
    {
        const long cx = static_cast<long>(cellOf(p.x - m_minX));
        const long cy = static_cast<long>(cellOf(p.y - m_minY));
        double best = std::numeric_limits<double>::infinity();
        for (long r = 0; r < static_cast<long>(N); r++) {
            if (best < static_cast<double>(r - 1) * m_cell) {
                break;
            }
            for (long y = cy - r; y <= cy + r; y++) {
                for (long x = cx - r; x <= cx + r; x++) {
                    if (std::max(std::labs(x - cx), std::labs(y - cy)) != r) {
                        continue;
                    }
                    if (x < 0 || y < 0 || x >= static_cast<long>(N) || y >= static_cast<long>(N)) {
                        continue;
                    }
                    for (std::size_t i : m_cells[static_cast<std::size_t>(y) * N + static_cast<std::size_t>(x)]) {
                        best = std::min(best, pointSegmentDistance(p, m_line[i], m_line[i + 1]));
                    }
                }
            }
        }
        return best;
    }

private:
    static constexpr std::size_t N = 128;

    std::size_t cellOf(double offset) const
    {
        const double c = std::floor(offset / m_cell);
        return static_cast<std::size_t>(std::min(static_cast<double>(N - 1), std::max(0.0, c)));
    }

    const std::vector<CoordinateXY>& m_line;
    double m_minX, m_minY, m_cell;
    std::vector<std::vector<std::size_t>> m_cells;
};

// Largest distance from points on `a` to the polyline `b`.
double
oneWay(const std::vector<CoordinateXY>& a, const std::vector<CoordinateXY>& b)
{
    const PolylineIndex index(b);
    double dev = 0.0;
    for (const CoordinateXY& p : a) {
        dev = std::max(dev, index.distance(p));
    }
    return dev;
}

std::vector<CoordinateXY>
polylineOf(const geos::geom::Geometry& g)
{
    std::vector<CoordinateXY> pts;
    const geos::geom::CoordinateSequence& cs = *static_cast<const geos::geom::LineString&>(g).getCoordinatesRO();
    for (std::size_t i = 0; i < cs.size(); i++) {
        pts.push_back(CoordinateXY(cs.getX(i), cs.getY(i)));
    }
    return pts;
}

CoordinateXY
startTangent(const NativeCurveSegment& s)
{
    return sub(s.control1, s.start);
}

CoordinateXY
endTangent(const NativeCurveSegment& s)
{
    return sub(s.end, s.control2);
}

} // namespace

namespace tut {

// ensure_equals(actual, expected, epsilon) without a message.
inline void
ensure_equals(double actual, double expected, double epsilon)
{
    ensure_equals("value", actual, expected, epsilon);
}

struct test_nativecurvesegments_data {
    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader reader_;

    std::unique_ptr<geos::geom::Geometry> read(const std::string& wkt) const
    {
        return reader_.read(wkt);
    }

    static std::string clothoidWkt(double scale, double t0, double t1)
    {
        return "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
               "(VECTOR (1 0), VECTOR (0 1))), SCALEFACTOR " + std::to_string(scale)
               + ", STARTDISTANCE " + std::to_string(t0) + ", ENDDISTANCE " + std::to_string(t1) + ")";
    }

    static std::string spiralWkt(const std::string& type, double length, double k0, double k1)
    {
        return "SPIRALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (10 20), REFERENCEDIRECTIONS "
               "(VECTOR (0.6 0.8), VECTOR (-0.8 0.6))), LENGTH " + std::to_string(length)
               + ", STARTCURVATURE " + std::to_string(k0) + ", ENDCURVATURE " + std::to_string(k1)
               + ", SPIRALTYPE " + type + ")";
    }

    static NurbsPoint weighted(double x, double y, double w)
    {
        NurbsPoint cp;
        cp.weightedPoint = Coordinate(w * x, w * y);
        cp.weight = w;
        return cp;
    }

    static NurbsPoint plain(double x, double y)
    {
        NurbsPoint cp;
        cp.weightedPoint = Coordinate(x, y);
        return cp;
    }

    NativeCurvePath convert(const geos::geom::Geometry& g, double tol = 0.001, bool g2 = true) const
    {
        NativeCurveParams params;
        params.tolerance = tol;
        params.matchCurvature = g2;
        const auto paths = NativeCurveSegments::convert(g, params);
        ensure_equals("one path", paths.size(), 1u);
        return paths[0];
    }

    // Two-way distance between the decoded path and a dense polyline of the
    // source curve.
    static double hausdorff(const NativeCurvePath& path, const std::vector<CoordinateXY>& ref,
                            std::size_t perSegment = 6000)
    {
        const std::vector<CoordinateXY> dec = decodePath(path, perSegment);
        return std::max(oneWay(dec, ref), oneWay(ref, dec));
    }

    static void ensureConnected(const NativeCurvePath& path)
    {
        for (std::size_t i = 1; i < path.segments.size(); i++) {
            ensure_equals("junction x", path.segments[i].start.x, path.segments[i - 1].end.x);
            ensure_equals("junction y", path.segments[i].start.y, path.segments[i - 1].end.y);
        }
    }

    // Tangent continuity at every junction between two Beziers, to a
    // relative 1e-9 in angle.
    static void ensureTangentContinuity(const NativeCurvePath& path)
    {
        for (std::size_t i = 1; i < path.segments.size(); i++) {
            const NativeCurveSegment& a = path.segments[i - 1];
            const NativeCurveSegment& b = path.segments[i];
            if (a.type != Type::CUBIC_BEZIER || b.type != Type::CUBIC_BEZIER) {
                continue;
            }
            const CoordinateXY ta = endTangent(a);
            const CoordinateXY tb = startTangent(b);
            ensure("tangents point the same way", ta.x * tb.x + ta.y * tb.y > 0);
            ensure("tangent continuity", std::abs(cross(ta, tb)) / (len(ta) * len(tb)) < 1e-9);
        }
    }

    // Signed curvature of a Bezier at its start or end.
    static double bezierCurvature(const NativeCurveSegment& s, bool atEnd)
    {
        CoordinateXY d1, d2;
        if (!atEnd) {
            d1 = CoordinateXY(3 * (s.control1.x - s.start.x), 3 * (s.control1.y - s.start.y));
            d2 = CoordinateXY(6 * (s.control2.x - 2 * s.control1.x + s.start.x),
                              6 * (s.control2.y - 2 * s.control1.y + s.start.y));
        }
        else {
            d1 = CoordinateXY(3 * (s.end.x - s.control2.x), 3 * (s.end.y - s.control2.y));
            d2 = CoordinateXY(6 * (s.end.x - 2 * s.control2.x + s.control1.x),
                              6 * (s.end.y - 2 * s.control2.y + s.control1.y));
        }
        return cross(d1, d2) / std::pow(len(d1), 3);
    }

    // Deviation of an approximation against the dense linearisation of the
    // source curve, and the reported bound.
    void ensureWithinTolerance(const geos::geom::Geometry& g, double tol, const std::string& label,
                               std::size_t* segmentCount = nullptr, bool g2 = true)
    {
        const NativeCurvePath path = convert(g, tol, g2);
        ensure(label + ": has segments", !path.segments.empty());
        const auto dense = g.getLinearized(CurveToLineParams::maxDeviation(tol * 1e-2));
        const std::vector<CoordinateXY> ref = polylineOf(*dense);
        const double dev = hausdorff(path, ref);
        {
            std::ostringstream msg;
            msg << std::setprecision(10) << label << ": deviation " << dev << " <= " << tol;
            ensure(msg.str(), dev <= tol);
        }
        double reported = 0.0;
        for (const NativeCurveSegment& s : path.segments) {
            ensure(label + ": approximation is marked", !s.exact);
            ensure(label + ": cubic Bezier", s.type == Type::CUBIC_BEZIER);
            reported = std::max(reported, s.maxDeviation);
        }
        ensure(label + ": reported deviation within tolerance", reported <= tol);
        ensure(label + ": reported deviation is positive", reported > 0.0);
        // The reported figure is a measurement of the same distance.
        ensure(label + ": reported ~ measured", dev <= reported * 1.1 + tol * 1e-3);
        ensureConnected(path);
        ensureTangentContinuity(path);
        if (segmentCount) {
            *segmentCount = path.segments.size();
        }
    }
};

typedef test_group<test_nativecurvesegments_data> group;
typedef group::object object;

group test_nativecurvesegments_group("geos::algorithm::NativeCurveSegments");

template<>
template<>
void object::test<1>()
{
    set_test_name("CircularString maps to circular arcs, exactly");

    auto g = read("CIRCULARSTRING (0 0, 1 1, 2 0, 3 -1, 4 0)");
    const NativeCurvePath path = convert(*g);
    ensure_equals(path.segments.size(), 2u);
    const NativeCurveSegment& a = path.segments[0];
    ensure(a.type == Type::CIRCULAR_ARC);
    ensure(a.exact);
    ensure_equals(a.maxDeviation, 0.0);
    ensure_equals(a.start.x, 0.0);
    ensure_equals(a.end.x, 2.0);
    ensure_equals(a.interior.x, 1.0);
    ensure_equals(a.interior.y, 1.0);
    ensure_equals(a.center.x, 1.0, 1e-14);
    ensure_equals(a.center.y, 0.0, 1e-14);
    ensure_equals(a.radius, 1.0, 1e-14);
    ensure(a.clockwise);
    const NativeCurveSegment& b = path.segments[1];
    ensure(!b.clockwise);
    ensure_equals(b.center.x, 3.0, 1e-14);
    ensure_equals(b.radius, 1.0, 1e-14);
    ensureConnected(path);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("CircularString: a complete circle is two arcs, a collinear arc is a line");

    auto circle = read("CIRCULARSTRING (0 0, 2 0, 0 0)");
    const NativeCurvePath p = convert(*circle);
    ensure_equals(p.segments.size(), 2u);
    for (const NativeCurveSegment& s : p.segments) {
        ensure(s.type == Type::CIRCULAR_ARC);
        ensure_equals(s.center.x, 1.0, 1e-14);
        ensure_equals(s.center.y, 0.0, 1e-14);
        ensure_equals(s.radius, 1.0, 1e-14);
        // Each half circle is a half circle: end and start are opposite.
        ensure_equals(len(sub(s.start, s.end)), 2.0, 1e-13);
    }
    ensure_equals(p.segments[0].end.x, p.segments[1].start.x);
    ensure_equals(p.segments[1].end.x, 0.0);
    const std::vector<CoordinateXY> dec = decodePath(p, 400);
    for (const CoordinateXY& q : dec) {
        ensure_equals("on the circle", len(sub(q, CoordinateXY(1, 0))), 1.0, 1e-12);
    }

    auto line = read("CIRCULARSTRING (0 0, 1 1, 2 2)");
    const NativeCurvePath l = convert(*line);
    ensure_equals(l.segments.size(), 1u);
    ensure(l.segments[0].type == Type::LINE);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("EllipticalCurve: circle arcs are circular arcs with centre and radius");

    auto g = read("ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (5 -3), "
                  "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 4, VAXISLENGTH 4, "
                  "STARTANGLE ANGLE RADIANS (0), ENDANGLE ANGLE RADIANS (1.5707963267948966))");
    const NativeCurvePath p = convert(*g);
    ensure_equals(p.segments.size(), 1u);
    const NativeCurveSegment& s = p.segments[0];
    ensure(s.type == Type::CIRCULAR_ARC);
    ensure(s.exact);
    ensure_equals(s.center.x, 5.0, 1e-14);
    ensure_equals(s.center.y, -3.0, 1e-14);
    ensure_equals(s.radius, 4.0, 1e-14);
    ensure_equals(s.start.x, 9.0, 1e-14);
    ensure_equals(s.start.y, -3.0, 1e-14);
    ensure_equals(s.end.x, 5.0, 1e-14);
    ensure_equals(s.end.y, 1.0, 1e-14);
    ensure_equals(s.interior.x, 5.0 + 4.0 * std::cos(PI / 4), 1e-13);
    ensure_equals(s.interior.y, -3.0 + 4.0 * std::sin(PI / 4), 1e-13);
    ensure(!s.clockwise);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("EllipticalCurve: ellipses, rotated, sheared, reflected and clockwise");

    struct Case {
        std::string name;
        std::string u, v;
        double a, b, t0, t1;
        std::size_t pieces;
    };
    const std::vector<Case> cases = {
        {"axis aligned half", "VECTOR (1 0)", "VECTOR (0 1)", 3, 2, 0.3, 0.3 + PI, 2},
        {"full ellipse", "VECTOR (1 0)", "VECTOR (0 1)", 3, 2, 0, 2 * PI, 3},
        {"rotated", "VECTOR (0.6 0.8)", "VECTOR (-0.8 0.6)", 3, 1, -1, 2, 2},
        {"sheared", "VECTOR (1 0)", "VECTOR (1 1)", 2, 1.5, 0.2, 5, 3},
        {"reflected", "VECTOR (1 0)", "VECTOR (0 -1)", 3, 2, 0, 4, 2},
        {"clockwise", "VECTOR (1 0)", "VECTOR (0 1)", 3, 2, 3, -1, 2},
        {"circle from scaled axes", "VECTOR (2 0)", "VECTOR (0 2)", 1.5, 1.5, 0.5, 2.5, 1}
    };
    for (const Case& c : cases) {
        const std::string wkt = "ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (1 2), "
                                "REFERENCEDIRECTIONS (" + c.u + ", " + c.v + ")), UAXISLENGTH "
                                + std::to_string(c.a) + ", VAXISLENGTH " + std::to_string(c.b)
                                + ", STARTANGLE ANGLE RADIANS (" + std::to_string(c.t0)
                                + "), ENDANGLE ANGLE RADIANS (" + std::to_string(c.t1) + "))";
        auto g = read(wkt);
        const auto* ec = static_cast<const geos::geom::EllipticalCurve*>(g.get());
        const NativeCurvePath p = convert(*g);
        ensure_equals(c.name + ": pieces", p.segments.size(), c.pieces);
        ensureConnected(p);
        // The start and end are the curve's own points.
        ensure_equals(c.name + ": start x", p.segments.front().start.x, ec->getCoordinatesRO()->getX(0));
        ensure_equals(c.name + ": end y", p.segments.back().end.y, ec->getCoordinatesRO()->getY(1));
        // The decoded arcs lie on the source curve, point for point.
        std::vector<CoordinateXY> ref;
        const std::size_t steps = 40000;
        for (std::size_t i = 0; i <= steps; i++) {
            const double t = c.t0 + (c.t1 - c.t0) * static_cast<double>(i) / static_cast<double>(steps);
            const auto pt = ec->getPointAtAngle(t);
            ref.push_back(CoordinateXY(pt.x, pt.y));
        }
        ensure(c.name + ": arcs match the curve", hausdorff(p, ref, 2000) < 1e-6);
        for (const NativeCurveSegment& s : p.segments) {
            ensure(c.name + ": exact", s.exact);
            // Orientation of the pieces agrees with the direction of travel.
            const bool ccw = cross(sub(s.interior, s.start), sub(s.end, s.interior)) > 0;
            ensure(c.name + ": direction", ccw == !s.clockwise);
        }
        if (c.name == "circle from scaled axes") {
            ensure(p.segments[0].type == Type::CIRCULAR_ARC);
            ensure_equals(p.segments[0].radius, 3.0, 1e-13);
        }
        else {
            ensure(p.segments[0].type == Type::ELLIPTIC_ARC);
            const NativeCurveSegment& s = p.segments[0];
            ensure(s.axisRatio > 0 && s.axisRatio <= 1.0);
            ensure(s.minor);
        }
    }
}

template<>
template<>
void object::test<5>()
{
    set_test_name("EllipticalCurve: axes and ratio of an axis-aligned and a rotated ellipse");

    auto g = read("ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (1 2), "
                  "REFERENCEDIRECTIONS (VECTOR (0.6 0.8), VECTOR (-0.8 0.6))), UAXISLENGTH 3, "
                  "VAXISLENGTH 1, STARTANGLE ANGLE RADIANS (0), ENDANGLE ANGLE RADIANS (1))");
    const NativeCurvePath p = convert(*g);
    ensure_equals(p.segments.size(), 1u);
    const NativeCurveSegment& s = p.segments[0];
    ensure_equals(s.center.x, 1.0, 1e-14);
    ensure_equals(s.center.y, 2.0, 1e-14);
    ensure_equals(s.semiMajorAxis, 3.0, 1e-13);
    ensure_equals(s.axisRatio, 1.0 / 3.0, 1e-13);
    // Rotation of the major axis, modulo pi.
    ensure_equals(std::fmod(std::atan2(0.8, 0.6) - s.rotation + 2 * PI, PI) < 1e-9
                  || PI - std::fmod(std::atan2(0.8, 0.6) - s.rotation + 2 * PI, PI) < 1e-9, true);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("NURBS: rational conics are arcs, exactly");

    const double w = std::sqrt(0.5);
    auto quarter = factory_->createNurbsCurve(
                       2, {weighted(1, 0, 1), weighted(1, 1, w), weighted(0, 1, 1)},
                       {NurbsKnot{0.0, 3}, NurbsKnot{1.0, 3}}, false, false);
    NativeCurvePath p = convert(*quarter);
    ensure_equals(p.segments.size(), 1u);
    const NativeCurveSegment& s = p.segments[0];
    ensure(s.type == Type::CIRCULAR_ARC);
    ensure(s.exact);
    ensure_equals(s.center.x, 0.0, 1e-14);
    ensure_equals(s.center.y, 0.0, 1e-14);
    ensure_equals(s.radius, 1.0, 1e-14);
    ensure_equals(s.start.x, 1.0, 1e-14);
    ensure_equals(s.end.y, 1.0, 1e-14);
    ensure_equals(s.interior.x, std::cos(PI / 4), 1e-13);
    ensure_equals(s.interior.y, std::sin(PI / 4), 1e-13);
    ensure(!s.clockwise);

    // The full circle: four quarter arcs.
    auto full = factory_->createNurbsCurve(
                    2, {weighted(1, 0, 1), weighted(1, 1, w), weighted(0, 1, 1),
                        weighted(-1, 1, w), weighted(-1, 0, 1), weighted(-1, -1, w),
                        weighted(0, -1, 1), weighted(1, -1, w), weighted(1, 0, 1)},
                    {NurbsKnot{0.0, 3}, NurbsKnot{0.25, 2}, NurbsKnot{0.5, 2},
                     NurbsKnot{0.75, 2}, NurbsKnot{1.0, 3}}, false, false);
    p = convert(*full);
    ensure_equals(p.segments.size(), 4u);
    for (const NativeCurveSegment& a : p.segments) {
        ensure(a.type == Type::CIRCULAR_ARC);
        ensure(a.exact);
        ensure_equals(a.radius, 1.0, 1e-13);
        ensure_equals(a.center.x, 0.0, 1e-13);
        ensure_equals(a.center.y, 0.0, 1e-13);
    }
    ensureConnected(p);
    ensure_equals(p.segments.front().start.x, p.segments.back().end.x, 1e-14);

    // An ellipse: the circle scaled by 3 along x.
    auto ellipse = full->affineTransform(3, 0, 0, 0, 1, 0);
    p = convert(*ellipse);
    ensure_equals(p.segments.size(), 4u);
    for (const NativeCurveSegment& a : p.segments) {
        ensure(a.type == Type::ELLIPTIC_ARC);
        ensure(a.exact);
        ensure_equals(a.semiMajorAxis, 3.0, 1e-13);
        ensure_equals(a.axisRatio, 1.0 / 3.0, 1e-13);
        ensure_equals(a.center.x, 0.0, 1e-13);
    }
    std::vector<CoordinateXY> ref;
    for (int i = 0; i <= 40000; i++) {
        const auto q = static_cast<const NurbsCurve*>(ellipse.get())->getPointAtParameter(i / 40000.0);
        ref.push_back(CoordinateXY(q.x, q.y));
    }
    ensure("ellipse arcs match the NURBS", hausdorff(p, ref, 2000) < 1e-6);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("NURBS: non-rational degree <= 3 is cubic Beziers, exactly, by knot insertion");

    // Cubic B-spline with interior knots (clamped).
    auto cubic = factory_->createNurbsCurve(
                     3, {plain(0, 0), plain(1, 2), plain(3, 3), plain(4, 0), plain(6, -1), plain(7, 2), plain(9, 1)},
                     {NurbsKnot{0.0, 4}, NurbsKnot{0.3, 1}, NurbsKnot{0.7, 2}, NurbsKnot{1.0, 4}}, false, false);
    // Quadratic, unclamped ends.
    auto quad = factory_->createNurbsCurve(
                    2, {plain(0, 0), plain(2, 3), plain(4, 0), plain(6, 3), plain(8, 0), plain(10, 3)},
                    {NurbsKnot{0.0, 1}, NurbsKnot{1.0, 1}, NurbsKnot{2.0, 1}, NurbsKnot{3.0, 1},
                     NurbsKnot{4.0, 1}, NurbsKnot{5.0, 1}, NurbsKnot{6.0, 1}, NurbsKnot{7.0, 1},
                     NurbsKnot{8.0, 1}}, false, false);
    // Linear.
    auto lin = factory_->createNurbsCurve(
                   1, {plain(0, 0), plain(1, 1), plain(3, 0)},
                   {NurbsKnot{0.0, 2}, NurbsKnot{0.5, 1}, NurbsKnot{1.0, 2}}, false, false);

    const std::vector<const NurbsCurve*> curves = {cubic.get(), quad.get(), lin.get()};
    const std::vector<std::size_t> expectedSegments = {3, 4, 2};
    const std::vector<Type> expectedType = {Type::CUBIC_BEZIER, Type::CUBIC_BEZIER, Type::LINE};
    for (std::size_t k = 0; k < curves.size(); k++) {
        const NurbsCurve* nc = curves[k];
        const NativeCurvePath p = convert(*nc);
        ensure_equals("segment count " + std::to_string(k), p.segments.size(), expectedSegments[k]);
        ensureConnected(p);
        for (const NativeCurveSegment& s : p.segments) {
            ensure("exact", s.exact);
            ensure_equals("no deviation", s.maxDeviation, 0.0);
            ensure("type", s.type == expectedType[k]);
        }
        std::vector<CoordinateXY> ref;
        const double u0 = nc->getStartParameter();
        const double u1 = nc->getEndParameter();
        for (int i = 0; i <= 60000; i++) {
            const auto q = nc->getPointAtParameter(u0 + (u1 - u0) * i / 60000.0);
            ref.push_back(CoordinateXY(q.x, q.y));
        }
        ensure("matches the NURBS " + std::to_string(k), hausdorff(p, ref, 2000) < 1e-6);
        ensure_equals("start", p.segments.front().start.x, nc->getCoordinatesRO()->getX(0));
        ensure_equals("end", p.segments.back().end.x, nc->getCoordinatesRO()->getX(1));
    }
}

template<>
template<>
void object::test<8>()
{
    set_test_name("NURBS: parabola span is exact, hyperbola and degree 4 are approximations");

    // Weights 1, 2, 4: w1^2 = w0 w2, a parabola.
    auto parabola = factory_->createNurbsCurve(
                        2, {weighted(0, 0, 1), weighted(1, 2, 2), weighted(2, 0, 4)},
                        {NurbsKnot{0.0, 3}, NurbsKnot{1.0, 3}}, false, false);
    NativeCurvePath p = convert(*parabola);
    ensure_equals(p.segments.size(), 1u);
    ensure(p.segments[0].type == Type::CUBIC_BEZIER);
    ensure(p.segments[0].exact);

    // Weight 2 at the corner of a right angle: a hyperbola arc.
    auto hyperbola = factory_->createNurbsCurve(
                         2, {weighted(1, 0, 1), weighted(1, 1, 2), weighted(0, 1, 1)},
                         {NurbsKnot{0.0, 3}, NurbsKnot{1.0, 3}}, false, false);
    p = convert(*hyperbola, 1e-4);
    ensure(!p.segments.empty());
    double dev = 0.0;
    for (const NativeCurveSegment& s : p.segments) {
        ensure(!s.exact);
        ensure(s.type == Type::CUBIC_BEZIER);
        dev = std::max(dev, s.maxDeviation);
    }
    ensure(dev <= 1e-4);
    std::vector<CoordinateXY> ref;
    for (int i = 0; i <= 40000; i++) {
        const auto q = static_cast<const NurbsCurve*>(hyperbola.get())->getPointAtParameter(i / 40000.0);
        ref.push_back(CoordinateXY(q.x, q.y));
    }
    ensure("hyperbola within tolerance", hausdorff(p, ref, 3000) <= 1e-4);

    // Degree 4, non-rational and not a cubic.
    auto quartic = factory_->createNurbsCurve(
                       4, {plain(0, 0), plain(1, 3), plain(2, -2), plain(3, 3), plain(4, 0)},
                       {NurbsKnot{0.0, 5}, NurbsKnot{1.0, 5}}, false, false);
    p = convert(*quartic, 1e-4);
    for (const NativeCurveSegment& s : p.segments) {
        ensure(!s.exact);
    }
    ref.clear();
    for (int i = 0; i <= 40000; i++) {
        const auto q = static_cast<const NurbsCurve*>(quartic.get())->getPointAtParameter(i / 40000.0);
        ref.push_back(CoordinateXY(q.x, q.y));
    }
    ensure("quartic within tolerance", hausdorff(p, ref, 3000) <= 1e-4);
    ensureConnected(p);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("Clothoid: Bezier chains within tolerance, several parameters and tolerances");

    struct Case {
        double scale, t0, t1;
    };
    const std::vector<Case> cases = {
        {50, 0, 25},      // the 25 m transition at A = 50
        {50, 0, 100},
        {10, 0, 40},      // eight radians of turn
        {80, -40, 60},    // through the inflection
        {30, 5, 45},      // does not start at the inflection
        {50, 100, 0},     // reversed: curvature decreasing
        {50, 25, -25},
    };
    for (const double tol : {1e-3, 1e-5}) {
        for (const Case& c : cases) {
            auto g = read(clothoidWkt(c.scale, c.t0, c.t1));
            ensureWithinTolerance(*g, tol, "clothoid A=" + std::to_string(c.scale) + " ["
                                  + std::to_string(c.t0) + ", " + std::to_string(c.t1) + "] tol "
                                  + std::to_string(tol));
        }
    }
}

template<>
template<>
void object::test<10>()
{
    set_test_name("SpiralCurve: all five types, reversed and with decreasing curvature");

    const std::vector<std::string> types = {"clothoid", "bloss", "biquadratic", "sine", "cosine"};
    for (const double tol : {1e-3, 1e-5}) {
        for (const std::string& type : types) {
            const std::string label = type + " tol " + std::to_string(tol);
            auto up = read(spiralWkt(type, 120, 0.0, 0.01));
            ensureWithinTolerance(*up, tol, label + " rising");
            // Curvature decreasing, non-zero start.
            auto down = read(spiralWkt(type, 120, 0.02, 0.004));
            ensureWithinTolerance(*down, tol, label + " falling");
            // Curvature changing sign.
            auto flip = read(spiralWkt(type, 100, -0.01, 0.015));
            ensureWithinTolerance(*flip, tol, label + " sign change");
            // The reversed curve.
            auto reversed = static_cast<const geos::geom::SpiralCurve*>(up.get())->reverse();
            ensureWithinTolerance(*reversed, tol, label + " reversed");
        }
    }
}

template<>
template<>
void object::test<11>()
{
    set_test_name("SpiralCurve: placement scaling and mirroring");

    // A non-orthogonal, scaled, mirrored placement.
    auto g = read("SPIRALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (3 4), REFERENCEDIRECTIONS "
                  "(VECTOR (2 0.5), VECTOR (0.3 -1.5))), LENGTH 60, STARTCURVATURE 0.001, "
                  "ENDCURVATURE 0.02, SPIRALTYPE bloss)");
    ensureWithinTolerance(*g, 1e-3, "affine spiral");
    auto c = read("CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (3 4), REFERENCEDIRECTIONS "
                  "(VECTOR (2 0.5), VECTOR (0.3 -1.5))), SCALEFACTOR 20, STARTDISTANCE 0, ENDDISTANCE 30)");
    ensureWithinTolerance(*c, 1e-3, "affine clothoid");
}

template<>
template<>
void object::test<12>()
{
    set_test_name("Bezier segment counts are small");

    std::size_t n = 0;
    auto c25 = read(clothoidWkt(50, 0, 25));
    ensureWithinTolerance(*c25, 1e-3, "25 m clothoid", &n);
    ensure("25 m clothoid at 1 mm takes at most 2 Beziers", n <= 2);

    auto c100 = read(clothoidWkt(50, 0, 100));
    ensureWithinTolerance(*c100, 1e-3, "100 m clothoid", &n);
    ensure("100 m clothoid at 1 mm takes at most 6 Beziers", n <= 6);
    ensureWithinTolerance(*c100, 1e-5, "100 m clothoid, 0.01 mm", &n);
    ensure("100 m clothoid at 0.01 mm takes at most 12 Beziers", n <= 12);

    auto bloss = read(spiralWkt("bloss", 100, 0, 0.01));
    ensureWithinTolerance(*bloss, 1e-3, "bloss transition", &n);
    ensure("100 m bloss transition at 1 mm takes at most 4 Beziers", n <= 4);

    // A tighter tolerance never needs fewer segments.
    std::size_t loose = 0, tight = 0;
    ensureWithinTolerance(*c100, 1e-2, "loose", &loose);
    ensureWithinTolerance(*c100, 1e-6, "tight", &tight);
    ensure(loose <= tight);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("Curvature is matched at the Bezier joints (G2) when asked");

    auto g = read(clothoidWkt(50, 0, 100));
    const NativeCurvePath p = convert(*g, 1e-3, true);
    ensure(p.segments.size() > 1);
    for (std::size_t i = 1; i < p.segments.size(); i++) {
        const double kEnd = bezierCurvature(p.segments[i - 1], true);
        const double kStart = bezierCurvature(p.segments[i], false);
        ensure_equals("curvature jump at joint " + std::to_string(i), kEnd, kStart,
                      1e-6 * std::abs(kEnd));
        // And it is the clothoid's own curvature t / A^2 at that point.
    }
    // The first Bezier starts straight and the last one ends at 100 / 2500.
    ensure_equals(bezierCurvature(p.segments.front(), false), 0.0, 1e-9);
    ensure_equals(bezierCurvature(p.segments.back(), true), 100.0 / 2500.0, 1e-6);

    // G1 only: still within tolerance, joints still tangent continuous.
    ensureWithinTolerance(*g, 1e-3, "G1 only", nullptr, false);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("CompoundCurve: sections join, ends are the curve's ends");

    auto start = read("LINESTRING (0 0, 10 0)");
    (void) start;
    auto g = read("COMPOUNDCURVE ((0 0, 10 0), CIRCULARSTRING (10 0, 12 2, 10 4), "
                  "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (10 4), REFERENCEDIRECTIONS "
                  "(VECTOR (1 0), VECTOR (0 1))), SCALEFACTOR 5, STARTDISTANCE 0, ENDDISTANCE 8))");
    const auto* cc = static_cast<const geos::geom::CompoundCurve*>(g.get());
    const NativeCurvePath p = convert(*g, 1e-3);
    ensure(p.segments.size() >= 3);
    ensureConnected(p);
    ensure(p.segments[0].type == Type::LINE);
    ensure(p.segments[1].type == Type::CIRCULAR_ARC);
    for (std::size_t i = 2; i < p.segments.size(); i++) {
        ensure(p.segments[i].type == Type::CUBIC_BEZIER);
    }
    const auto* last = cc->getCurveN(cc->getNumCurves() - 1);
    const geos::geom::CoordinateSequence& cs = *last->getCoordinatesRO();
    ensure_equals(p.segments.back().end.x, cs.getX(cs.size() - 1));
    ensure_equals(p.segments.back().end.y, cs.getY(cs.size() - 1));
    ensure_equals(p.segments.front().start.x, 0.0);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("Polygon rings are clockwise shells and counterclockwise holes");

    auto g = read("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 5 5, 10 0), (10 0, 0 0)), "
                  "(3 1, 5 1, 5 2, 3 2, 3 1))");
    NativeCurveParams params;
    const auto paths = NativeCurveSegments::convert(*g, params);
    ensure_equals(paths.size(), 2u);
    ensure(paths[0].closed);
    ensure(paths[1].closed);
    auto area = [](const NativeCurvePath& path) {
        const std::vector<CoordinateXY> pts = decodePath(path, 200);
        double a = 0;
        for (std::size_t i = 0; i + 1 < pts.size(); i++) {
            a += cross(pts[i], pts[i + 1]);
        }
        return 0.5 * a;
    };
    ensure("shell clockwise", area(paths[0]) < 0);
    ensure("hole counterclockwise", area(paths[1]) > 0);
    ensureConnected(paths[0]);
    ensure_equals(paths[0].segments.front().start.x, paths[0].segments.back().end.x);
    ensure_equals(paths[0].segments.front().start.y, paths[0].segments.back().end.y);
}

template<>
template<>
void object::test<16>()
{
    set_test_name("Errors: bad tolerance and unsupported geometry");

    auto g = read(clothoidWkt(50, 0, 25));
    NativeCurveParams params;
    params.tolerance = 0.0;
    ensure_THROW(NativeCurveSegments::convert(*g, params), geos::util::IllegalArgumentException);
    params.tolerance = -1.0;
    ensure_THROW(NativeCurveSegments::convert(*g, params), geos::util::IllegalArgumentException);
    params.tolerance = std::numeric_limits<double>::quiet_NaN();
    ensure_THROW(NativeCurveSegments::convert(*g, params), geos::util::IllegalArgumentException);
    params.tolerance = 1e-3;
    auto coll = read("GEOMETRYCOLLECTION (POINT (1 1), LINESTRING (0 0, 1 1))");
    ensure_THROW(NativeCurveSegments::convert(*coll, params), geos::util::UnsupportedOperationException);
    // A point has no path.
    auto pt = read("POINT (1 1)");
    ensure(NativeCurveSegments::convert(*pt, params).empty());
    // Empty curves have no path.
    auto empty = read("CLOTHOID EMPTY");
    ensure(NativeCurveSegments::convert(*empty, params).empty());
    // A singular placement is refused.
    auto flat = read("CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
                     "(VECTOR (1 0), VECTOR (2 0))), SCALEFACTOR 5, STARTDISTANCE 0, ENDDISTANCE 8)");
    ensure_THROW(NativeCurveSegments::convert(*flat, params), geos::util::UnsupportedOperationException);
}

template<>
template<>
void object::test<17>()
{
    set_test_name("CurveToLine is unchanged by the conversion");

    auto g = read(clothoidWkt(50, 0, 25));
    const auto before = g->getLinearized(CurveToLineParams::maxDeviation(1e-3));
    NativeCurveParams params;
    (void) NativeCurveSegments::convert(*g, params);
    const auto after = g->getLinearized(CurveToLineParams::maxDeviation(1e-3));
    ensure(before->equalsExact(after.get(), 0.0));
}

} // namespace tut
