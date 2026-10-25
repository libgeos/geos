#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/algorithm/CurveToLineParams.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/constants.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/NurbsCurve.h>
#include <geos/geom/Point.h>
#include <geos/io/ByteOrderValues.h>
#include <geos/io/ParseException.h>
#include <geos/io/WKBConstants.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

using geos::algorithm::CurveToLineParams;
using geos::algorithm::PointLocator;
using geos::geom::CompoundCurve;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYZM;
using geos::geom::NurbsCurve;
using geos::geom::NurbsKnot;
using geos::geom::NurbsPoint;
using geos::geom::SimpleCurve;

namespace tut {

struct test_nurbscurve_data {
    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader reader_;

    const double w_ = std::sqrt(0.5);

    // A control point at cartesian (x, y) with weight w.
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

    static std::vector<NurbsKnot> clamped(int degree, std::vector<double> interior = {})
    {
        std::vector<NurbsKnot> knots;
        knots.push_back(NurbsKnot{0.0, static_cast<std::size_t>(degree + 1)});
        for (double value : interior) {
            knots.push_back(NurbsKnot{value, 1});
        }
        knots.push_back(NurbsKnot{1.0, static_cast<std::size_t>(degree + 1)});
        return knots;
    }

    // The unit quarter circle from (1, 0) to (0, 1).
    std::unique_ptr<NurbsCurve> quarterCircle() const
    {
        return factory_->createNurbsCurve(
                   2, {weighted(1, 0, 1), weighted(1, 1, w_), weighted(0, 1, 1)},
                   clamped(2), false, false);
    }

    // The unit circle from (1, 0), counterclockwise.
    std::unique_ptr<NurbsCurve> fullCircle() const
    {
        std::vector<NurbsPoint> cps = {
            weighted(1, 0, 1), weighted(1, 1, w_), weighted(0, 1, 1),
            weighted(-1, 1, w_), weighted(-1, 0, 1), weighted(-1, -1, w_),
            weighted(0, -1, 1), weighted(1, -1, w_), weighted(1, 0, 1)};
        std::vector<NurbsKnot> knots = {
            {0.0, 3}, {0.25, 2}, {0.5, 2}, {0.75, 2}, {1.0, 3}};
        return factory_->createNurbsCurve(2, std::move(cps), std::move(knots), false, false);
    }

    static void ensureOnUnitCircle(const CoordinateXY& c, double cx = 0.0, double cy = 0.0)
    {
        ensure_equals("radius", std::hypot(c.x - cx, c.y - cy), 1.0, 1e-14);
    }
};

typedef test_group<test_nurbscurve_data> group;
typedef group::object object;

group test_nurbscurve_group("geos::geom::NurbsCurve");

template<>
template<>
void object::test<1>()
{
    set_test_name("type id and empty");

    ensure_equals(static_cast<int>(geos::geom::GEOS_NURBSCURVE), 13);
    auto empty = factory_->createNurbsCurve(false, false);
    ensure(empty->isEmpty());
    ensure_equals(empty->getGeometryType(), std::string("NurbsCurve"));
    ensure_equals(empty->getGeometryTypeId(), geos::geom::GEOS_NURBSCURVE);
    ensure_equals(empty->getLength(), 0.0);
    ensure(empty->getEnvelopeInternal()->isNull());
    ensure(!empty->hasCurvedComponents());
    ensure(empty->hasCurvedTypes());
    ensure(factory_->createEmpty(geos::geom::GEOS_NURBSCURVE)->getGeometryTypeId()
           == geos::geom::GEOS_NURBSCURVE);
    ensure(empty->getLinearized(CurveToLineParams())->isEmpty());
    ensure_THROW(empty->getPointAtParameter(0.0), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("rational quarter circle is exact");

    auto arc = quarterCircle();
    ensure(arc->isRational());
    ensure_equals(arc->getDegree(), 2);
    ensure_equals(arc->getStartParameter(), 0.0);
    ensure_equals(arc->getEndParameter(), 1.0);
    ensure(arc->getStartPoint()->getCoordinate()->equals2D(CoordinateXY(1, 0)));
    ensure(arc->getEndPoint()->getCoordinate()->equals2D(CoordinateXY(0, 1)));
    for (int i = 0; i <= 64; ++i) {
        ensureOnUnitCircle(arc->getPointAtParameter(i / 64.0));
    }
    ensure_equals("length", arc->getLength(), geos::MATH_PI / 2.0, 1e-14);
    const auto* env = arc->getEnvelopeInternal();
    ensure_equals("envelope", env->getMinX(), 0.0, 1e-15);
    ensure_equals("envelope", env->getMinY(), 0.0, 1e-15);
    ensure_equals("envelope", env->getMaxX(), 1.0, 1e-15);
    ensure_equals("envelope", env->getMaxY(), 1.0, 1e-15);
    ensure(arc->hasCurvedComponents());
    ensure(!arc->isClosed());
}

template<>
template<>
void object::test<3>()
{
    set_test_name("full circle is closed with length 2 pi");

    auto circle = fullCircle();
    ensure(circle->isClosed());
    ensure_equals("length", circle->getLength(), 2.0 * geos::MATH_PI, 1e-13);
    for (int i = 0; i <= 100; ++i) {
        ensureOnUnitCircle(circle->getPointAtParameter(i / 100.0));
    }
    const auto* env = circle->getEnvelopeInternal();
    ensure_equals("envelope", env->getMinX(), -1.0, 1e-15);
    ensure_equals("envelope", env->getMaxX(), 1.0, 1e-15);
    ensure_equals("envelope", env->getMinY(), -1.0, 1e-15);
    ensure_equals("envelope", env->getMaxY(), 1.0, 1e-15);
    ensure(circle->getBoundary()->isEmpty());
}

template<>
template<>
void object::test<4>()
{
    set_test_name("clamped cubic matches its Bernstein form");

    const CoordinateXY p[] = {{0, 0}, {1, 3}, {3, -1}, {4, 2}};
    auto bezier = factory_->createNurbsCurve(
                      3, {plain(0, 0), plain(1, 3), plain(3, -1), plain(4, 2)},
                      clamped(3), false, false);
    ensure(!bezier->isRational());
    ensure(bezier->getStartPoint()->getCoordinate()->equals2D(p[0]));
    ensure(bezier->getEndPoint()->getCoordinate()->equals2D(p[3]));
    for (int i = 0; i <= 10; ++i) {
        const double t = i / 10.0;
        const double s = 1.0 - t;
        const double b[] = {s * s * s, 3 * s * s * t, 3 * s * t * t, t * t * t};
        double x = 0.0;
        double y = 0.0;
        for (int j = 0; j < 4; ++j) {
            x += b[j] * p[j].x;
            y += b[j] * p[j].y;
        }
        const CoordinateXYZM c = bezier->getPointAtParameter(t);
        ensure_equals("x", c.x, x, 1e-14);
        ensure_equals("y", c.y, y, 1e-14);
        ensure(std::isnan(c.z));
        ensure(std::isnan(c.m));
    }
}

template<>
template<>
void object::test<5>()
{
    set_test_name("unclamped uniform cubic starts and ends inside its hull");

    std::vector<NurbsKnot> knots;
    for (int i = 0; i < 8; ++i) {
        knots.push_back(NurbsKnot{static_cast<double>(i), 1});
    }
    auto spline = factory_->createNurbsCurve(
                      3, {plain(0, 0), plain(1, 2), plain(3, 2), plain(4, 0)},
                      std::move(knots), false, false);
    ensure_equals(spline->getStartParameter(), 3.0);
    ensure_equals(spline->getEndParameter(), 4.0);
    // (P0 + 4 P1 + P2) / 6 and (P1 + 4 P2 + P3) / 6.
    const auto& start = spline->getCoordinatesRO()->getAt<CoordinateXY>(0);
    const auto& end = spline->getCoordinatesRO()->getAt<CoordinateXY>(1);
    ensure_equals("endpoint", start.x, 7.0 / 6.0, 1e-15);
    ensure_equals("endpoint", start.y, 10.0 / 6.0, 1e-15);
    ensure_equals("endpoint", end.x, 17.0 / 6.0, 1e-15);
    ensure_equals("endpoint", end.y, 10.0 / 6.0, 1e-15);
    // Parameters outside the domain are clamped to it.
    ensure(spline->getPointAtParameter(-5.0).equals2D(start));
    ensure(spline->getPointAtParameter(50.0).equals2D(end));
}

template<>
template<>
void object::test<6>()
{
    set_test_name("invalid construction is rejected");

    using geos::util::IllegalArgumentException;
    auto make = [&](int degree, std::vector<NurbsPoint> cps, std::vector<NurbsKnot> knots) {
        return factory_->createNurbsCurve(degree, std::move(cps), std::move(knots), false, false);
    };
    const std::vector<NurbsPoint> three = {plain(0, 0), plain(1, 1), plain(2, 0)};

    ensure_THROW(make(0, three, {{0, 1}, {1, 1}, {2, 1}, {3, 1}}), IllegalArgumentException);
    ensure_THROW(make(3, three, clamped(3)), IllegalArgumentException);
    ensure_THROW(make(NurbsCurve::MAX_DEGREE + 1, three, clamped(2)), IllegalArgumentException);
    // Knot count is not n + p + 1.
    ensure_THROW(make(2, three, {{0, 3}, {1, 2}}), IllegalArgumentException);
    // Knot values not strictly increasing.
    ensure_THROW(make(2, three, {{0, 3}, {0, 1}, {1, 2}}), IllegalArgumentException);
    ensure_THROW(make(2, three, {{1, 3}, {0, 3}}), IllegalArgumentException);
    // Zero multiplicity, and multiplicity above degree + 1.
    ensure_THROW(make(2, three, {{0, 3}, {0.5, 0}, {1, 3}}), IllegalArgumentException);
    ensure_THROW(make(1, three, {{0, 3}, {1, 2}}), IllegalArgumentException);
    // Interior multiplicity above the degree would break the curve.
    ensure_THROW(make(1, {plain(0, 0), plain(1, 1), plain(2, 0), plain(3, 1)},
                      {{0, 2}, {0.5, 2}, {1, 2}}),
                 IllegalArgumentException);
    // Mixed, zero, negative and non-finite weights.
    ensure_THROW(make(2, {weighted(0, 0, 1), plain(1, 1), weighted(2, 0, 1)}, clamped(2)),
                 IllegalArgumentException);
    ensure_THROW(make(2, {weighted(0, 0, 1), weighted(1, 1, 0), weighted(2, 0, 1)}, clamped(2)),
                 IllegalArgumentException);
    ensure_THROW(make(2, {weighted(0, 0, 1), weighted(1, 1, -1), weighted(2, 0, 1)}, clamped(2)),
                 IllegalArgumentException);
    // Non-finite coordinates and knots.
    ensure_THROW(make(2, {plain(0, 0), plain(geos::DoubleInfinity, 1), plain(2, 0)}, clamped(2)),
                 IllegalArgumentException);
    ensure_THROW(make(2, three, {{0, 3}, {geos::DoubleNotANumber, 3}}), IllegalArgumentException);
    // Non-finite measures.
    ensure_THROW(factory_->createNurbsCurve(2, three, clamped(2), false, true,
                                            0.0, geos::DoubleNotANumber),
                 IllegalArgumentException);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("empty knot domain is rejected");

    // Knots -1, 0, 0, 0, 1, 1: the domain [u2, u3] is [0, 0].
    ensure_THROW(factory_->createNurbsCurve(
                     2, {plain(0, 0), plain(1, 1), plain(2, 0)},
                     {{-1, 1}, {0, 3}, {1, 2}}, false, false),
                 geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<8>()
{
    set_test_name("WKT round trip");

    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(17);
    std::vector<std::unique_ptr<NurbsCurve>> curves;
    curves.push_back(quarterCircle());
    curves.push_back(fullCircle());
    for (const auto& curve : curves) {
        const std::string text = writer.write(curve.get());
        ensure(text.rfind("NURBSCURVE (DEGREE 2, CONTROLPOINTS (NURBSPOINT (WEIGHTEDPOINT (", 0) == 0);
        auto back = reader_.read<NurbsCurve>(text);
        ensure(curve->equalsIdentical(back.get()));
    }

    const std::string zm =
        "NURBSCURVE ZM (DEGREE 1, CONTROLPOINTS Z ("
        "NURBSPOINT (WEIGHTEDPOINT Z (0 0 0), WEIGHT 1), "
        "NURBSPOINT (WEIGHTEDPOINT Z (2 2 2), WEIGHT 2)), "
        "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2)), "
        "STARTM 0, ENDM 10)";
    auto curve = reader_.read<NurbsCurve>(zm);
    ensure(curve->hasZ());
    ensure(curve->hasM());
    ensure_equals(writer.write(curve.get()), zm);
    const auto* seq = curve->getCoordinatesRO();
    ensure_equals(seq->getX(1), 1.0);
    ensure_equals(seq->getY(1), 1.0);
    ensure_equals(seq->getZ(1), 1.0);
    ensure_equals(seq->getM(0), 0.0);
    ensure_equals(seq->getM(1), 10.0);
    ensure_equals(curve->getStartPoint()->getM(), 0.0);
    ensure_equals(curve->getEndPoint()->getM(), 10.0);

    ensure_equals(writer.write(reader_.read("NURBSCURVE EMPTY").get()), std::string("NURBSCURVE EMPTY"));
    ensure_equals(writer.write(reader_.read("NURBSCURVE ZM EMPTY").get()), std::string("NURBSCURVE ZM EMPTY"));
    ensure_equals(writer.write(reader_.read("nurbscurve m (degree 1, controlpoints ("
                                            "nurbspoint (weightedpoint (0 0), weight 1), "
                                            "nurbspoint (weightedpoint (1 0), weight 1)), "
                                            "knots (knot (value 0, multiplicity 2), "
                                            "knot (value 1, multiplicity 2)), startm 3, endm 4)").get()),
                  std::string("NURBSCURVE M (DEGREE 1, CONTROLPOINTS ("
                              "NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), "
                              "NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                              "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), "
                              "KNOT (VALUE 1, MULTIPLICITY 2)), STARTM 3, ENDM 4)"));
}

template<>
template<>
void object::test<9>()
{
    set_test_name("rejected WKT");

    using geos::io::ParseException;
    const std::string body =
        "(DEGREE 1, CONTROLPOINTS (NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), "
        "NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
        "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2))";
    ensure(reader_.read("NURBSCURVE " + body + ")") != nullptr);
    // STARTM/ENDM only with M, and required with M.
    ensure_THROW(reader_.read("NURBSCURVE " + body + ", STARTM 0, ENDM 1)"), ParseException);
    ensure_THROW(reader_.read("NURBSCURVE M " + body + ")"), ParseException);
    // CONTROLPOINTS and WEIGHTEDPOINT carry Z exactly when the curve does.
    ensure_THROW(reader_.read("NURBSCURVE Z " + body + ")"), ParseException);
    ensure_THROW(reader_.read(
                     "NURBSCURVE (DEGREE 1, CONTROLPOINTS Z (NURBSPOINT (WEIGHTEDPOINT Z (0 0 0), WEIGHT 1), "
                     "NURBSPOINT (WEIGHTEDPOINT Z (1 0 0), WEIGHT 1)), "
                     "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2)))"),
                 ParseException);
    // Every NURBSPOINT has a WEIGHT.
    ensure_THROW(reader_.read(
                     "NURBSCURVE (DEGREE 1, CONTROLPOINTS (NURBSPOINT (WEIGHTEDPOINT (0 0)), "
                     "NURBSPOINT (WEIGHTEDPOINT (1 0))), "
                     "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2)))"),
                 ParseException);
    // DEGREE and MULTIPLICITY are integers.
    ensure_THROW(reader_.read(
                     "NURBSCURVE (DEGREE 1.5, CONTROLPOINTS (NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), "
                     "NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                     "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2)))"),
                 ParseException);
    ensure_THROW(reader_.read(
                     "NURBSCURVE (DEGREE 1, CONTROLPOINTS (NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), "
                     "NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                     "KNOTS (KNOT (VALUE 0, MULTIPLICITY -2), KNOT (VALUE 1, MULTIPLICITY 2)))"),
                 ParseException);
    // Empty parts of a non-empty curve.
    ensure_THROW(reader_.read(
                     "NURBSCURVE (DEGREE 1, CONTROLPOINTS EMPTY, KNOTS EMPTY)"),
                 ParseException);
    ensure_THROW(reader_.read(
                     "NURBSCURVE (DEGREE 1, CONTROLPOINTS (NURBSPOINT EMPTY, "
                     "NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                     "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2)))"),
                 ParseException);
    // A well-formed text that is not a valid curve.
    ensure_THROW(reader_.read(
                     "NURBSCURVE (DEGREE 1, CONTROLPOINTS (NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), "
                     "NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                     "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 1)))"),
                 geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("WKB layout");

    // Degree 1, (0 0) to (1 0), non-rational, clamped on [0, 1].
    const char* hexLiteral =
        "0115000000"
        "01"
        "02000000"
        "01" "0000000000000000" "0000000000000000" "00"
        "01" "000000000000F03F" "0000000000000000" "00"
        "02000000"
        "01" "0000000000000000" "02"
        "01" "000000000000F03F" "02";
    auto curve = factory_->createNurbsCurve(
                     1, {plain(0, 0), plain(1, 0)}, clamped(1), false, false);
    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*curve, hex);
    ensure_equals(hex.str(), std::string(hexLiteral));

    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream hexIn(hexLiteral);
    auto back = wkbReader.readHEX(hexIn);
    ensure(curve->equalsIdentical(back.get()));
    ensure(!static_cast<const NurbsCurve&>(*back).isRational());
}

template<>
template<>
void object::test<11>()
{
    set_test_name("WKB round trip in every dimension and byte order");

    geos::io::WKTWriter wktWriter;
    wktWriter.setRoundingPrecision(17);
    const std::string zm =
        "NURBSCURVE ZM (DEGREE 2, CONTROLPOINTS Z ("
        "NURBSPOINT (WEIGHTEDPOINT Z (0 0 1), WEIGHT 1), "
        "NURBSPOINT (WEIGHTEDPOINT Z (1 2 3), WEIGHT 0.5), "
        "NURBSPOINT (WEIGHTEDPOINT Z (4 0 2), WEIGHT 2)), "
        "KNOTS (KNOT (VALUE 0, MULTIPLICITY 3), KNOT (VALUE 1, MULTIPLICITY 3)), "
        "STARTM 5, ENDM 7)";
    auto curve = reader_.read(zm);

    for (int flavor : {geos::io::WKBConstants::wkbExtended, geos::io::WKBConstants::wkbIso}) {
        for (int order : {geos::io::ByteOrderValues::ENDIAN_LITTLE, geos::io::ByteOrderValues::ENDIAN_BIG}) {
            for (int dim = 2; dim <= 4; ++dim) {
                geos::io::WKBWriter writer(static_cast<uint8_t>(dim), order, false, flavor);
                std::stringstream hex;
                writer.writeHEX(*curve, hex);
                geos::io::WKBReader wkbReader(*factory_);
                auto back = wkbReader.readHEX(hex);
                ensure_equals(back->getGeometryTypeId(), geos::geom::GEOS_NURBSCURVE);
                if (dim == 4) {
                    ensure(curve->equalsIdentical(back.get()));
                }
                else {
                    // Dropping M, and Z for 2, keeps the plan curve.
                    ensure_equals(back->hasZ(), dim == 3);
                    ensure(!back->hasM());
                    const auto& nurbs = static_cast<const NurbsCurve&>(*back);
                    for (int i = 0; i <= 8; ++i) {
                        const CoordinateXYZM a = static_cast<const NurbsCurve&>(*curve).getPointAtParameter(i / 8.0);
                        ensure(nurbs.getPointAtParameter(i / 8.0).equals2D(a));
                    }
                }
            }
        }
    }

    geos::io::WKBWriter isoWriter(
        4, geos::io::ByteOrderValues::ENDIAN_LITTLE, false, geos::io::WKBConstants::wkbIso);
    std::stringstream isoZM;
    isoWriter.writeHEX(*curve, isoZM);
    ensure(isoZM.str().rfind("01CD0B0000", 0) == 0);
}

template<>
template<>
void object::test<12>()
{
    set_test_name("WKB empty, alone and before another geometry");

    geos::io::WKBWriter wkbWriter;
    std::stringstream emptyHex;
    wkbWriter.writeHEX(*factory_->createNurbsCurve(false, false), emptyHex);
    ensure_equals(emptyHex.str(), std::string("0115000000"));

    geos::io::WKBWriter isoWriter(
        4, geos::io::ByteOrderValues::ENDIAN_LITTLE, false, geos::io::WKBConstants::wkbIso);
    std::stringstream isoZ;
    isoWriter.writeHEX(*factory_->createNurbsCurve(true, false), isoZ);
    ensure_equals(isoZ.str(), std::string("01FD030000"));
    std::stringstream isoM;
    isoWriter.writeHEX(*factory_->createNurbsCurve(false, true), isoM);
    ensure_equals(isoM.str(), std::string("01E5070000"));

    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream emptyStream("0115000000");
    ensure(wkbReader.readHEX(emptyStream)->isEmpty());

    // The empty curve is followed by geometries in each byte order, the
    // little endian one starting with the byte a degree 1 body starts with.
    for (int order : {geos::io::ByteOrderValues::ENDIAN_LITTLE, geos::io::ByteOrderValues::ENDIAN_BIG}) {
        for (const char* wkt : {
                    "MULTICURVE (NURBSCURVE EMPTY, (0 0, 1 1))",
                    "MULTICURVE (NURBSCURVE EMPTY, NURBSCURVE (DEGREE 1, CONTROLPOINTS ("
                    "NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                    "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2))))",
                    "MULTICURVE (NURBSCURVE (DEGREE 1, CONTROLPOINTS ("
                    "NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                    "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2))), NURBSCURVE EMPTY)"
                }) {
            auto multi = reader_.read(wkt);
            geos::io::WKBWriter writer(2, order);
            std::stringstream hex;
            writer.writeHEX(*multi, hex);
            auto back = wkbReader.readHEX(hex);
            ensure(wkt, multi->equalsIdentical(back.get()));
        }
    }

    // Degree 0 is not a curve, and is not taken for a following geometry.
    std::stringstream degreeZero("011500000000");
    ensure_THROW(wkbReader.readHEX(degreeZero), geos::io::ParseException);
    // A weight flag other than 0 or 1.
    std::stringstream badFlag(
        "0115000000" "02" "01000000"
        "01" "0000000000000000" "0000000000000000" "02");
    ensure_THROW(wkbReader.readHEX(badFlag), geos::io::ParseException);
    // A truncated body.
    std::stringstream truncated("0115000000" "02" "03000000");
    ensure_THROW(wkbReader.readHEX(truncated), geos::io::ParseException);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("linearization honors deviation and step");

    auto arc = quarterCircle();
    for (double deviation : {1e-1, 1e-3, 1e-6}) {
        auto line = arc->getLinearized(CurveToLineParams::maxDeviation(deviation));
        const auto* seq = line->getCoordinatesRO();
        ensure(seq->getAt<CoordinateXY>(0).equals2D(CoordinateXY(1, 0)));
        ensure(seq->back<CoordinateXY>().equals2D(CoordinateXY(0, 1)));
        for (std::size_t i = 1; i < seq->size(); ++i) {
            const CoordinateXY& a = seq->getAt<CoordinateXY>(i - 1);
            const CoordinateXY& b = seq->getAt<CoordinateXY>(i);
            ensureOnUnitCircle(b);
            // Sagitta of a unit-circle chord.
            const double half = 0.5 * a.distance(b);
            ensure(1.0 - std::sqrt(1.0 - half * half) <= deviation);
        }
    }

    auto stepped = arc->getLinearized(CurveToLineParams::stepSizeDegrees(4.0));
    const auto* seq = stepped->getCoordinatesRO();
    ensure(seq->size() >= 1 + 90 / 4);
    for (std::size_t i = 1; i < seq->size(); ++i) {
        const CoordinateXY& a = seq->getAt<CoordinateXY>(i - 1);
        const CoordinateXY& b = seq->getAt<CoordinateXY>(i);
        const double angle = std::abs(std::atan2(b.y, b.x) - std::atan2(a.y, a.x));
        ensure(angle <= 4.0 * geos::MATH_PI / 180.0 + 1e-12);
    }
}

template<>
template<>
void object::test<14>()
{
    set_test_name("linearized M runs linearly in length");

    auto line = reader_.read<NurbsCurve>(
                    "NURBSCURVE M (DEGREE 1, CONTROLPOINTS ("
                    "NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), "
                    "NURBSPOINT (WEIGHTEDPOINT (10 0), WEIGHT 1)), "
                    "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2)), "
                    "STARTM 0, ENDM 10)");
    auto linear = line->getLinearized(CurveToLineParams());
    const auto* seq = linear->getCoordinatesRO();
    ensure(seq->hasM());
    for (std::size_t i = 0; i < seq->size(); ++i) {
        ensure_equals("m", seq->getM(i), seq->getX(i), 1e-12);
    }
    ensure_equals(seq->getM(0), 0.0);
    ensure_equals(seq->getM(seq->size() - 1), 10.0);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("reverse and normalize");

    auto circle = fullCircle();
    auto reversed = circle->reverse();
    for (int i = 0; i <= 20; ++i) {
        const double u = i / 20.0;
        const CoordinateXYZM a = circle->getPointAtParameter(u);
        const CoordinateXYZM b = reversed->getPointAtParameter(-u);
        ensure_equals("reversed point", a.x, b.x, 1e-14);
        ensure_equals("reversed point", a.y, b.y, 1e-14);
    }
    ensure(reversed->reverse()->equalsIdentical(circle.get()));

    auto arc = quarterCircle();
    auto back = arc->reverse();
    ensure(back->getStartPoint()->getCoordinate()->equals2D(CoordinateXY(0, 1)));
    ensure(back->getEndPoint()->getCoordinate()->equals2D(CoordinateXY(1, 0)));
    ensure_equals("length", back->getLength(), arc->getLength(), 1e-14);

    // normalize puts the smaller endpoint first.
    auto normalized = arc->clone();
    normalized->normalize();
    ensure(normalized->equalsIdentical(back.get()));
    auto unchanged = back->clone();
    unchanged->normalize();
    ensure(unchanged->equalsIdentical(back.get()));

    auto measured = reader_.read<NurbsCurve>(
                        "NURBSCURVE M (DEGREE 1, CONTROLPOINTS ("
                        "NURBSPOINT (WEIGHTEDPOINT (0 0), WEIGHT 1), "
                        "NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1)), "
                        "KNOTS (KNOT (VALUE 0, MULTIPLICITY 2), KNOT (VALUE 1, MULTIPLICITY 2)), "
                        "STARTM 3, ENDM 4)");
    auto measuredBack = measured->reverse();
    ensure_equals(measuredBack->getStartM(), 4.0);
    ensure_equals(measuredBack->getEndM(), 3.0);
}

template<>
template<>
void object::test<16>()
{
    set_test_name("affine image is exact");

    const double angle = 0.4;
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    auto circle = fullCircle();
    circle->setSRID(4326);
    auto image = circle->affineTransform(c, -s, 10.0, s, c, 20.0);
    ensure_equals(image->getSRID(), 4326);
    for (int i = 0; i <= 50; ++i) {
        ensureOnUnitCircle(image->getPointAtParameter(i / 50.0), 10.0, 20.0);
    }
    // The extremes are inside the spans now, so the envelope is found by
    // subdivision.
    const auto* env = image->getEnvelopeInternal();
    ensure_equals("envelope", env->getMinX(), 9.0, 1e-13);
    ensure_equals("envelope", env->getMaxX(), 11.0, 1e-13);
    ensure_equals("envelope", env->getMinY(), 19.0, 1e-13);
    ensure_equals("envelope", env->getMaxY(), 21.0, 1e-13);
    ensure_equals("length", image->getLength(), 2.0 * geos::MATH_PI, 1e-13);
    ensure_THROW(circle->affineTransform(geos::DoubleInfinity, 0, 0, 0, 1, 0),
                 geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<17>()
{
    set_test_name("envelope covers the curve tightly");

    std::vector<NurbsKnot> knots = {{0, 4}, {0.3, 1}, {0.5, 2}, {0.8, 1}, {1, 4}};
    auto spline = factory_->createNurbsCurve(
                      3,
    {weighted(0, 0, 1), weighted(2, 5, 3), weighted(4, -3, 0.5), weighted(6, 4, 2),
     weighted(8, -2, 1), weighted(9, 3, 0.25), weighted(10, 0, 1), weighted(11, 2, 1)},
    std::move(knots), false, false);
    const auto* env = spline->getEnvelopeInternal();
    geos::geom::Envelope sampled;
    for (int i = 0; i <= 20000; ++i) {
        const CoordinateXYZM p = spline->getPointAtParameter(i / 20000.0);
        ensure(env->covers(p.x, p.y));
        sampled.expandToInclude(p.x, p.y);
    }
    ensure(env->getMinX() >= sampled.getMinX() - 1e-6);
    ensure(env->getMaxX() <= sampled.getMaxX() + 1e-6);
    ensure(env->getMinY() >= sampled.getMinY() - 1e-6);
    ensure(env->getMaxY() <= sampled.getMaxY() + 1e-6);
}

template<>
template<>
void object::test<18>()
{
    set_test_name("CompoundCurve section");

    // A rational end whose cartesian point rounds when divided by its weight.
    std::vector<std::unique_ptr<SimpleCurve>> sections;
    sections.push_back(factory_->createNurbsCurve(
                           1, {weighted(0, 0, 1), weighted(0.7, 0.3, 0.3)}, clamped(1), false, false));
    sections.push_back(factory_->createLineString({CoordinateXY(0.7, 0.3), CoordinateXY(2, 2)}));
    auto compound = factory_->createCompoundCurve(std::move(sections));
    ensure(compound->getCurveN(0)->getCoordinatesRO()->back<CoordinateXY>().equals2D(CoordinateXY(0.7, 0.3)));

    auto text = geos::io::WKTWriter().write(compound.get());
    ensure(text.find("COMPOUNDCURVE (NURBSCURVE (DEGREE 1") == 0);
    auto back = reader_.read(text);
    ensure_equals(back->getGeometryTypeId(), geos::geom::GEOS_COMPOUNDCURVE);

    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*compound, hex);
    geos::io::WKBReader wkbReader(*factory_);
    auto fromWkb = wkbReader.readHEX(hex);
    ensure(compound->equalsIdentical(fromWkb.get()));

    std::vector<std::unique_ptr<SimpleCurve>> ring;
    ring.push_back(quarterCircle());
    ring.push_back(factory_->createLineString({CoordinateXY(0, 1), CoordinateXY(0, 0), CoordinateXY(1, 0)}));
    auto closed = factory_->createCompoundCurve(std::move(ring));
    ensure(closed->isClosed());
    auto linear = closed->getLinearized(CurveToLineParams::maxDeviation(1e-4));
    ensure(linear->isClosed());
    ensure_equals("length", closed->getLength(), geos::MATH_PI / 2.0 + 2.0, 1e-13);
}

template<>
template<>
void object::test<19>()
{
    set_test_name("operations that read vertices refuse a NurbsCurve");

    auto arc = quarterCircle();
    auto point = factory_->createPoint(CoordinateXY(0.0, 0.0));
    ensure_THROW(point->distance(arc.get()), geos::util::UnsupportedOperationException);
    auto line = factory_->createLineString({CoordinateXY(0, 0), CoordinateXY(2, 2)});
    ensure_THROW(arc->intersection(line.get()), geos::util::UnsupportedOperationException);
    PointLocator locator;
    ensure_THROW(locator.locate(CoordinateXY(1, 0), arc.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(arc->clone()->apply_rw(static_cast<const geos::geom::CoordinateFilter*>(nullptr)),
                 geos::util::UnsupportedOperationException);
}

template<>
template<>
void object::test<20>()
{
    set_test_name("equality and ordering");

    auto plainLine = factory_->createNurbsCurve(
                         1, {plain(0, 0), plain(1, 0)}, clamped(1), false, false);
    auto unitWeights = factory_->createNurbsCurve(
                           1, {weighted(0, 0, 1), weighted(1, 0, 1)}, clamped(1), false, false);
    // An absent weight is 1, but the representation differs.
    ensure(plainLine->equalsExact(unitWeights.get()));
    ensure(!plainLine->equalsIdentical(unitWeights.get()));
    ensure_equals(plainLine->compareTo(unitWeights.get()), 0);

    auto arc = quarterCircle();
    auto other = factory_->createNurbsCurve(
                     2, {weighted(1, 0, 1), weighted(1, 1, 0.5), weighted(0, 1, 1)},
                     clamped(2), false, false);
    ensure(!arc->equalsExact(other.get()));
    ensure(arc->compareTo(other.get()) != 0);
    ensure_equals(arc->compareTo(other.get()), -other->compareTo(arc.get()));
    ensure(arc->equalsIdentical(arc->clone().get()));
    ensure(!arc->equalsExact(plainLine.get()));
}

template<>
template<>
void object::test<21>()
{
    set_test_name("coincident control points trace a point");

    auto dot = factory_->createNurbsCurve(
                   2, {weighted(3, 4, 1), weighted(3, 4, 2), weighted(3, 4, 1)}, clamped(2), false, false);
    ensure(!dot->hasCurvedComponents());
    ensure_equals("length", dot->getLength(), 0.0, 1e-12);
    ensure(dot->isClosed());
    // Its tangents are rounding noise; linearizing must still stop.
    auto linear = dot->getLinearized(CurveToLineParams::stepSizeDegrees(1.0));
    ensure(linear->getNumPoints() <= 1 + 2 * 2);
    ensure(linear->getCoordinatesRO()->getAt<CoordinateXY>(0).equals2D(CoordinateXY(3, 4)));
}

} // namespace tut
