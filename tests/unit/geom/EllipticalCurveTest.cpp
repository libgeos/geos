#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/algorithm/CurveToLineParams.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/constants.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/EllipticalCurve.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
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
using geos::geom::AffinePlacement;
using geos::geom::CompoundCurve;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYZM;
using geos::geom::EllipticalCurve;
using geos::geom::SimpleCurve;

namespace tut {

struct test_ellipticalcurve_data {
    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader reader_;

    static constexpr double PI = geos::MATH_PI;

    static AffinePlacement identityAt(double x, double y, double z = 0.0)
    {
        return AffinePlacement{Coordinate(x, y, z), Coordinate(1, 0, 0), Coordinate(0, 1, 0)};
    }

    std::unique_ptr<EllipticalCurve> make(double a, double b, double t0, double t1,
                                          const AffinePlacement& at = identityAt(0, 0)) const
    {
        return factory_->createEllipticalCurve(at, a, b, t0, t1, false, false);
    }

    // a = 3, b = 2 ellipse from (3 0) to (0 2): the first quadrant.
    static constexpr const char* QUARTER_WKT =
        "ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 3, VAXISLENGTH 2, "
        "STARTANGLE ANGLE DEGREES (0), ENDANGLE ANGLE DEGREES (90))";
};

typedef test_group<test_ellipticalcurve_data> group;
typedef group::object object;

group test_ellipticalcurve_group("geos::geom::EllipticalCurve");

template<>
template<>
void object::test<1>()
{
    set_test_name("type id and empty");

    ensure_equals(static_cast<int>(geos::geom::GEOS_ELLIPTICALCURVE), 13);
    auto empty = factory_->createEllipticalCurve(false, false);
    ensure(empty->isEmpty());
    ensure_equals(empty->getGeometryType(), std::string("EllipticalCurve"));
    ensure_equals(empty->getLength(), 0.0);
    ensure(empty->getEnvelopeInternal()->isNull());
    ensure(!empty->isFullEllipse());
    ensure(empty->hasCurvedTypes());
    ensure(factory_->createEmpty(geos::geom::GEOS_ELLIPTICALCURVE)->getGeometryTypeId()
           == geos::geom::GEOS_ELLIPTICALCURVE);
    ensure(empty->getLinearized(CurveToLineParams())->isEmpty());
    ensure_THROW(empty->getPointAtAngle(0.0), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("quarter ellipse ends exactly on its axes");

    auto arc = reader_.read<EllipticalCurve>(QUARTER_WKT);
    ensure_equals(arc->getStartAngle(), 0.0);
    ensure_equals("end angle", arc->getEndAngle(), PI / 2, 1e-16);
    ensure(arc->getStartPoint()->getCoordinate()->equals2D(CoordinateXY(3, 0)));
    ensure(arc->getEndPoint()->getCoordinate()->equals2D(CoordinateXY(0, 2)));
    // a E(1 - b^2 / a^2), the complete elliptic integral of the second kind.
    ensure_equals("length", arc->getLength(), 3.966359897322647, 1e-13);
    const auto* env = arc->getEnvelopeInternal();
    ensure_equals(env->getMinX(), 0.0);
    ensure_equals(env->getMinY(), 0.0);
    ensure_equals(env->getMaxX(), 3.0);
    ensure_equals(env->getMaxY(), 2.0);
    ensure(!arc->isClosed());
    ensure(arc->hasCurvedComponents());
}

template<>
template<>
void object::test<3>()
{
    set_test_name("full ellipse is a closed ring");

    for (auto [t0, t1] : {std::pair<double, double>{0.0, 2 * PI}, {2 * PI, 0.0}, {-PI, PI}, {1.0, 1.0 + 2 * PI}}) {
        auto ellipse = make(3, 2, t0, t1);
        const std::string at = std::to_string(t0) + " -> " + std::to_string(t1);
        ensure("full " + at, ellipse->isFullEllipse());
        ensure("closed " + at, ellipse->isClosed());
        ensure("boundary " + at, ellipse->getBoundary()->isEmpty());
        ensure_equals("perimeter", ellipse->getLength(), 15.865439589290588, 1e-12);
        const auto* env = ellipse->getEnvelopeInternal();
        ensure_equals("envelope", env->getMinX(), -3.0, 1e-15);
        ensure_equals("envelope", env->getMaxX(), 3.0, 1e-15);
        ensure_equals("envelope", env->getMinY(), -2.0, 1e-15);
        ensure_equals("envelope", env->getMaxY(), 2.0, 1e-15);
    }
    auto degrees = reader_.read<EllipticalCurve>(
                       "ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                       "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 1, VAXISLENGTH 1, "
                       "STARTANGLE ANGLE DEGREES (0), ENDANGLE ANGLE DEGREES (360))");
    ensure("degrees closed", degrees->isClosed());
    ensure_equals("circle", degrees->getLength(), 2 * PI, 1e-13);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("sweep direction follows the angle order");

    // 7.6.1 Desc 21: 0 -> 90 is counterclockwise over 90 degrees, 90 -> 360
    // counterclockwise over 270, and swapping the angles reverses the arc.
    auto ccw = make(1, 1, 0, PI / 2);
    auto cw = make(1, 1, PI / 2, 0);
    auto long_way = make(1, 1, PI / 2, 2 * PI);
    ensure_equals("ccw", ccw->getLength(), PI / 2, 1e-14);
    ensure_equals("cw", cw->getLength(), PI / 2, 1e-14);
    ensure_equals("270", long_way->getLength(), 1.5 * PI, 1e-14);
    ensure(cw->getStartPoint()->getCoordinate()->equals2D(CoordinateXY(0, 1)));
    ensure(cw->getEndPoint()->getCoordinate()->equals2D(CoordinateXY(1, 0)));
    // The 270 degree arc reaches x = -1 and y = -1; the 90 degree one does not.
    ensure_equals(long_way->getEnvelopeInternal()->getMinX(), -1.0);
    ensure_equals(long_way->getEnvelopeInternal()->getMinY(), -1.0);
    ensure_equals(ccw->getEnvelopeInternal()->getMinX(), 0.0);
    ensure(cw->equalsExact(ccw->reverse().get()));
}

template<>
template<>
void object::test<5>()
{
    set_test_name("angle units");

    auto withAngles = [&](const std::string& start, const std::string& end) {
        return reader_.read<EllipticalCurve>(
                   "ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                   "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 3, VAXISLENGTH 2, "
                   "STARTANGLE ANGLE " + start + ", ENDANGLE ANGLE " + end + ")");
    };
    auto degrees = withAngles("DEGREES (0)", "DEGREES (90)");
    auto gradians = withAngles("GRADIANS (0)", "GRADIANS (100)");
    auto radians = withAngles("RADIANS (0)", "RADIANS (1.5707963267948966)");
    ensure(degrees->equalsExact(gradians.get(), 1e-15));
    ensure(degrees->equalsExact(radians.get(), 1e-15));
    ensure(radians->getEndPoint()->getCoordinate()->equals2D(CoordinateXY(0, 2)));

    // An arc of 1 radian, against a (E(m) - E(pi/2 - 1 | m)).
    auto one = withAngles("RADIANS (0)", "RADIANS (1)");
    ensure_equals("length", one->getLength(), 2.3036429919396886, 1e-13);
    ensure_equals("x", one->getEndPoint()->getX(), 3 * std::cos(1.0), 1e-15);
    ensure_equals("y", one->getEndPoint()->getY(), 2 * std::sin(1.0), 1e-15);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("invalid construction is rejected");

    using geos::util::IllegalArgumentException;
    ensure_THROW(make(0, 1, 0, 1), IllegalArgumentException);
    ensure_THROW(make(1, -1, 0, 1), IllegalArgumentException);
    ensure_THROW(make(geos::DoubleInfinity, 1, 0, 1), IllegalArgumentException);
    ensure_THROW(make(1, 1, geos::DoubleNotANumber, 1), IllegalArgumentException);
    ensure_THROW(make(1, 1, 0, 2 * PI + 1e-6), IllegalArgumentException);
    ensure_THROW(make(1, 1, -4, 4), IllegalArgumentException);
    ensure_THROW(make(1, 1, 0, 1, AffinePlacement{Coordinate(geos::DoubleInfinity, 0, 0),
                                                     Coordinate(1, 0, 0), Coordinate(0, 1, 0)}),
                 IllegalArgumentException);
    ensure_THROW(factory_->createEllipticalCurve(identityAt(0, 0), 1, 1, 0, 1, false, true,
                 0.0, geos::DoubleNotANumber),
                 IllegalArgumentException);
    // A full turn computed in degrees is still a full turn.
    ensure(make(1, 1, 0, 360 * (PI / 180))->isFullEllipse());
}

template<>
template<>
void object::test<7>()
{
    set_test_name("WKT round trip and form");

    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(17);
    const std::string zm =
        "ELLIPTICALCURVE ZM (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION Z (1 2 3), "
        "REFERENCEDIRECTIONS (VECTOR Z (0 1 0), VECTOR Z (-1 0 0))), UAXISLENGTH 5, VAXISLENGTH 2, "
        "STARTANGLE ANGLE RADIANS (0), ENDANGLE ANGLE RADIANS (2), STARTM 10, ENDM 20)";
    auto curve = reader_.read<EllipticalCurve>(zm);
    ensure(curve->hasZ());
    ensure(curve->hasM());
    ensure_equals(writer.write(curve.get()), zm);
    ensure(curve->getStartPoint()->getCoordinate()->equals2D(CoordinateXY(1, 7)));
    ensure_equals(curve->getStartPoint()->getZ(), 3.0);
    ensure_equals(curve->getStartPoint()->getM(), 10.0);
    ensure_equals(curve->getEndPoint()->getM(), 20.0);

    auto quarter = reader_.read<EllipticalCurve>(QUARTER_WKT);
    auto back = reader_.read<EllipticalCurve>(writer.write(quarter.get()));
    ensure(quarter->equalsIdentical(back.get()));

    ensure_equals(writer.write(reader_.read("ELLIPTICALCURVE EMPTY").get()),
                  std::string("ELLIPTICALCURVE EMPTY"));
    ensure_equals(writer.write(reader_.read("ellipticalcurve m empty").get()),
                  std::string("ELLIPTICALCURVE M EMPTY"));
}

template<>
template<>
void object::test<8>()
{
    set_test_name("rejected WKT");

    using geos::io::ParseException;
    const std::string placement =
        "(REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), ";
    const std::string tail =
        "UAXISLENGTH 3, VAXISLENGTH 2, STARTANGLE ANGLE DEGREES (0), ENDANGLE ANGLE DEGREES (90)";
    ensure(reader_.read("ELLIPTICALCURVE " + placement + tail + ")") != nullptr);
    ensure_THROW(reader_.read("ELLIPTICALCURVE " + placement + tail + ", STARTM 0, ENDM 1)"), ParseException);
    ensure_THROW(reader_.read("ELLIPTICALCURVE M " + placement + tail + ")"), ParseException);
    ensure_THROW(reader_.read("ELLIPTICALCURVE Z " + placement + tail + ")"), ParseException);
    ensure_THROW(reader_.read("ELLIPTICALCURVE " + placement +
                              "UAXISLENGTH EMPTY, VAXISLENGTH 2, STARTANGLE ANGLE DEGREES (0), "
                              "ENDANGLE ANGLE DEGREES (90))"), ParseException);
    ensure_THROW(reader_.read("ELLIPTICALCURVE " + placement +
                              "UAXISLENGTH 3, VAXISLENGTH 2, STARTANGLE ANGLE TURNS (0), "
                              "ENDANGLE ANGLE DEGREES (90))"), ParseException);
    ensure_THROW(reader_.read("ELLIPTICALCURVE " + placement +
                              "UAXISLENGTH 3, VAXISLENGTH 2, STARTANGLE 0, ENDANGLE 90)"), ParseException);
    // Exactly two reference directions.
    ensure_THROW(reader_.read("ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                              "REFERENCEDIRECTIONS (VECTOR (1 0))), " + tail + ")"), ParseException);
    // A well-formed text that is not a valid curve.
    ensure_THROW(reader_.read("ELLIPTICALCURVE " + placement +
                              "UAXISLENGTH 3, VAXISLENGTH 2, STARTANGLE ANGLE DEGREES (0), "
                              "ENDANGLE ANGLE DEGREES (720))"), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("WKB layout and round trips");

    // Unit circle quadrant: header, placement 102, a, b, start, end.
    const char* hexLiteral =
        "0114000000"
        "0166000000" "0000000000000000" "0000000000000000"
        "02000000" "000000000000F03F" "0000000000000000" "0000000000000000" "000000000000F03F"
        "000000000000F03F" "000000000000F03F"
        "0000000000000000" "182D4454FB21F93F";
    auto arc = make(1, 1, 0, PI / 2);
    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*arc, hex);
    ensure_equals(hex.str(), std::string(hexLiteral));
    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream hexIn(hexLiteral);
    ensure(arc->equalsIdentical(wkbReader.readHEX(hexIn).get()));

    auto zm = reader_.read(
                  "ELLIPTICALCURVE ZM (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION Z (1 2 3), "
                  "REFERENCEDIRECTIONS (VECTOR Z (0 1 0), VECTOR Z (-1 0 0))), UAXISLENGTH 5, VAXISLENGTH 2, "
                  "STARTANGLE ANGLE RADIANS (0), ENDANGLE ANGLE RADIANS (2), STARTM 10, ENDM 20)");
    for (int flavor : {geos::io::WKBConstants::wkbExtended, geos::io::WKBConstants::wkbIso}) {
        for (int order : {geos::io::ByteOrderValues::ENDIAN_LITTLE, geos::io::ByteOrderValues::ENDIAN_BIG}) {
            geos::io::WKBWriter writer(4, order, false, flavor);
            std::stringstream out;
            writer.writeHEX(*zm, out);
            ensure(zm->equalsIdentical(wkbReader.readHEX(out).get()));
        }
    }

    geos::io::WKBWriter isoWriter(
        4, geos::io::ByteOrderValues::ENDIAN_LITTLE, false, geos::io::WKBConstants::wkbIso);
    std::stringstream isoZM;
    isoWriter.writeHEX(*zm, isoZM);
    ensure(isoZM.str().rfind("01CC0B0000" "014E040000", 0) == 0);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("WKB empty, alone and before another geometry");

    geos::io::WKBWriter wkbWriter;
    std::stringstream emptyHex;
    wkbWriter.writeHEX(*factory_->createEllipticalCurve(false, false), emptyHex);
    ensure_equals(emptyHex.str(), std::string("0114000000"));

    geos::io::WKBReader wkbReader(*factory_);
    for (int order : {geos::io::ByteOrderValues::ENDIAN_LITTLE, geos::io::ByteOrderValues::ENDIAN_BIG}) {
        for (const char* wkt : {
                    "MULTICURVE (ELLIPTICALCURVE EMPTY, (0 0, 1 1))",
                    "MULTICURVE (ELLIPTICALCURVE EMPTY, ELLIPTICALCURVE EMPTY)",
                    "MULTICURVE (ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                    "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 3, VAXISLENGTH 2, "
                    "STARTANGLE ANGLE DEGREES (0), ENDANGLE ANGLE DEGREES (90)), ELLIPTICALCURVE EMPTY)"
                }) {
            auto multi = reader_.read(wkt);
            geos::io::WKBWriter writer(2, order);
            std::stringstream hex;
            writer.writeHEX(*multi, hex);
            ensure(wkt, multi->equalsIdentical(wkbReader.readHEX(hex).get()));
        }
    }
    // Three reference directions.
    std::stringstream three(
        "0114000000" "0166000000" "0000000000000000" "0000000000000000"
        "03000000" "000000000000F03F" "0000000000000000" "0000000000000000" "000000000000F03F"
        "0000000000000000" "0000000000000000");
    ensure_THROW(wkbReader.readHEX(three), geos::io::ParseException);
}

template<>
template<>
void object::test<11>()
{
    set_test_name("linearization honors deviation and step");

    auto circle = make(1, 1, 0, 2 * PI);
    for (double deviation : {1e-1, 1e-3, 1e-6}) {
        auto line = circle->getLinearized(CurveToLineParams::maxDeviation(deviation));
        const auto* seq = line->getCoordinatesRO();
        ensure(line->isClosed());
        for (std::size_t i = 1; i < seq->size(); ++i) {
            const CoordinateXY& a = seq->getAt<CoordinateXY>(i - 1);
            const CoordinateXY& b = seq->getAt<CoordinateXY>(i);
            ensure_equals("radius", std::hypot(b.x, b.y), 1.0, 1e-15);
            const double half = 0.5 * a.distance(b);
            ensure(1.0 - std::sqrt(1.0 - half * half) <= deviation);
        }
    }
    auto stepped = make(1, 1, 0, PI / 2)->getLinearized(CurveToLineParams::stepSizeDegrees(7));
    const auto* steps = stepped->getCoordinatesRO();
    ensure_equals(steps->size(), std::size_t(1 + 13));
    for (std::size_t i = 1; i < steps->size(); ++i) {
        const CoordinateXY& a = steps->getAt<CoordinateXY>(i - 1);
        const CoordinateXY& b = steps->getAt<CoordinateXY>(i);
        ensure(std::atan2(b.y, b.x) - std::atan2(a.y, a.x) <= 7 * PI / 180 + 1e-12);
    }

    auto measured = reader_.read<EllipticalCurve>(
                        "ELLIPTICALCURVE M (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 1, VAXISLENGTH 1, "
                        "STARTANGLE ANGLE DEGREES (0), ENDANGLE ANGLE DEGREES (90), STARTM 0, ENDM 9)");
    auto linear = measured->getLinearized(CurveToLineParams::stepSizeDegrees(7));
    const auto* seq = linear->getCoordinatesRO();
    const double n = static_cast<double>(seq->size() - 1);
    for (std::size_t i = 0; i < seq->size(); ++i) {
        ensure_equals("m", seq->getM(i), 9.0 * static_cast<double>(i) / n, 1e-12);
    }
}

template<>
template<>
void object::test<12>()
{
    set_test_name("affine image and tilted envelope");

    const double c = std::cos(0.5);
    const double s = std::sin(0.5);
    auto arc = make(3, 2, 0.3, 4.0);
    arc->setSRID(28992);
    auto image = arc->affineTransform(c, -s, 10, s, c, -5);
    ensure_equals(image->getSRID(), 28992);
    for (int i = 0; i <= 20; ++i) {
        const double t = 0.3 + 3.7 * i / 20.0;
        const CoordinateXYZM p = arc->getPointAtAngle(t);
        const CoordinateXYZM q = image->getPointAtAngle(t);
        ensure_equals("x", q.x, c * p.x - s * p.y + 10, 1e-14);
        ensure_equals("y", q.y, s * p.x + c * p.y - 5, 1e-14);
    }
    ensure_equals("length", image->getLength(), arc->getLength(), 1e-13);

    // A sheared, tilted placement: the envelope is exact against sampling.
    AffinePlacement tilted{Coordinate(5, 5, 0), Coordinate(0.8, 0.6, 0), Coordinate(-0.3, 0.9, 0)};
    auto sheared = make(4, 1.5, -1.0, 3.5, tilted);
    geos::geom::Envelope sampled;
    for (int i = 0; i <= 100000; ++i) {
        const CoordinateXYZM p = sheared->getPointAtAngle(-1.0 + 4.5 * i / 100000.0);
        ensure(sheared->getEnvelopeInternal()->covers(p.x, p.y));
        sampled.expandToInclude(p.x, p.y);
    }
    const auto* env = sheared->getEnvelopeInternal();
    ensure_equals("min x", env->getMinX(), sampled.getMinX(), 1e-8);
    ensure_equals("max x", env->getMaxX(), sampled.getMaxX(), 1e-8);
    ensure_equals("min y", env->getMinY(), sampled.getMinY(), 1e-8);
    ensure_equals("max y", env->getMaxY(), sampled.getMaxY(), 1e-8);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("reverse, normalize and equality");

    auto arc = make(3, 2, 0.25, 2.5);
    auto back = arc->reverse();
    ensure_equals(back->getStartAngle(), 2.5);
    ensure_equals(back->getEndAngle(), 0.25);
    ensure(back->reverse()->equalsIdentical(arc.get()));
    ensure_equals("length", back->getLength(), arc->getLength(), 1e-15);

    // normalize puts the smaller endpoint first.
    auto normalized = arc->clone();
    normalized->normalize();
    const bool startFirst = arc->getStartPoint()->getCoordinate()->compareTo(
                                *arc->getEndPoint()->getCoordinate()) <= 0;
    ensure(normalized->equalsIdentical(startFirst ? arc.get() : back.get()));

    ensure(arc->equalsIdentical(arc->clone().get()));
    ensure_equals(arc->compareTo(arc->clone().get()), 0);
    auto other = make(3, 2.5, 0.25, 2.5);
    ensure(!arc->equalsExact(other.get()));
    ensure_equals(arc->compareTo(other.get()), -other->compareTo(arc.get()));
    ensure(arc->compareTo(other.get()) != 0);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("CompoundCurve section and ring");

    // A stadium: two half circles and two lines, closing exactly because the
    // half circles end on their axes.
    std::vector<std::unique_ptr<SimpleCurve>> ring;
    ring.push_back(factory_->createLineString({CoordinateXY(0, -1), CoordinateXY(4, -1)}));
    ring.push_back(make(1, 1, -PI / 2, PI / 2, identityAt(4, 0)));
    ring.push_back(factory_->createLineString({CoordinateXY(4, 1), CoordinateXY(0, 1)}));
    ring.push_back(make(1, 1, PI / 2, 1.5 * PI, identityAt(0, 0)));
    auto stadium = factory_->createCompoundCurve(std::move(ring));
    ensure(stadium->isClosed());
    ensure_equals("length", stadium->getLength(), 8 + 2 * PI, 1e-13);

    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(17);
    const std::string text = writer.write(stadium.get());
    ensure(text.find("COMPOUNDCURVE ((0 -1, 4 -1), ELLIPTICALCURVE (") == 0);
    ensure(stadium->equalsIdentical(reader_.read(text).get()));

    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*stadium, hex);
    geos::io::WKBReader wkbReader(*factory_);
    ensure(stadium->equalsIdentical(wkbReader.readHEX(hex).get()));

    auto polygon = reader_.read("CURVEPOLYGON (" + text.substr(0) + ")");
    auto linear = polygon->getLinearized(CurveToLineParams::maxDeviation(1e-4));
    ensure_equals("area", linear->getArea(), 8 + PI, 1e-3);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("operations that read vertices refuse an EllipticalCurve");

    auto arc = make(3, 2, 0, PI);
    auto point = factory_->createPoint(CoordinateXY(0, 0));
    ensure_THROW(point->distance(arc.get()), geos::util::UnsupportedOperationException);
    auto line = factory_->createLineString({CoordinateXY(-5, 1), CoordinateXY(5, 1)});
    ensure_THROW(arc->intersection(line.get()), geos::util::UnsupportedOperationException);
    PointLocator locator;
    ensure_THROW(locator.locate(CoordinateXY(3, 0), arc.get()), geos::util::UnsupportedOperationException);
    ensure_THROW(arc->clone()->apply_rw(static_cast<const geos::geom::CoordinateFilter*>(nullptr)),
                 geos::util::UnsupportedOperationException);
}

} // namespace tut
