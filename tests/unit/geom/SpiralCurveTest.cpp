#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/algorithm/CurveToLineParams.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/constants.h>
#include <geos/geom/Clothoid.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/SpiralCurve.h>
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
using geos::geom::SimpleCurve;
using geos::geom::SpiralCurve;
using geos::geom::SpiralType;

namespace tut {

struct test_spiralcurve_data {
    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader reader_;

    static constexpr double PI = geos::MATH_PI;

    static AffinePlacement identityAt(double x, double y, double z = 0.0)
    {
        return AffinePlacement{Coordinate(x, y, z), Coordinate(1, 0, 0), Coordinate(0, 1, 0)};
    }

    std::unique_ptr<SpiralCurve> make(double length, double k0, double k1, SpiralType type,
                                      const AffinePlacement& at = identityAt(0, 0)) const
    {
        return factory_->createSpiralCurve(at, length, k0, k1, type, false, false);
    }

    static std::vector<SpiralType> allTypes()
    {
        return {SpiralType::CLOTHOID, SpiralType::BLOSS, SpiralType::BIQUADRATIC,
                SpiralType::SINE, SpiralType::COSINE};
    }

    // Composite Simpson integration of the unit tangent, as an independent
    // reference for the local point at the fraction t.
    static CoordinateXY reference(double length, double k0, double k1, SpiralType type, double t)
    {
        auto f = [type](double s) {
            switch (type) {
            case SpiralType::CLOTHOID: return s;
            case SpiralType::BLOSS: return 3 * s * s - 2 * s * s * s;
            case SpiralType::BIQUADRATIC: return s <= 0.5 ? 2 * s * s : 1 - 2 * (1 - s) * (1 - s);
            case SpiralType::SINE: return s - std::sin(2 * PI * s) / (2 * PI);
            case SpiralType::COSINE: return (1 - std::cos(PI * s)) / 2;
            }
            return s;
        };
        // Heading by Simpson as well, so nothing is shared with the code
        // under test but the definition of f.
        const int n = 20000;
        const double h = t / n;
        double x = 0.0;
        double y = 0.0;
        double theta = 0.0;
        double prevKappa = k0 + (k1 - k0) * f(0.0);
        for (int i = 0; i < n; ++i) {
            const double a = i * h;
            const double m = a + h / 2;
            const double b = a + h;
            const double km = k0 + (k1 - k0) * f(m);
            const double kb = k0 + (k1 - k0) * f(b);
            const double thetaM = theta + length * (h / 2) * (prevKappa + km) / 2;
            const double thetaB = theta + length * h * (prevKappa + 4 * km + kb) / 6;
            x += length * h * (std::cos(theta) + 4 * std::cos(thetaM) + std::cos(thetaB)) / 6;
            y += length * h * (std::sin(theta) + 4 * std::sin(thetaM) + std::sin(thetaB)) / 6;
            theta = thetaB;
            prevKappa = kb;
        }
        return CoordinateXY(x, y);
    }
};

typedef test_group<test_spiralcurve_data> group;
typedef group::object object;

group test_spiralcurve_group("geos::geom::SpiralCurve");

template<>
template<>
void object::test<1>()
{
    set_test_name("type id, names and empty");

    ensure_equals(static_cast<int>(geos::geom::GEOS_SPIRALCURVE), 14);
    auto empty = factory_->createSpiralCurve(false, false);
    ensure(empty->isEmpty());
    ensure_equals(empty->getGeometryType(), std::string("SpiralCurve"));
    ensure_equals(empty->getLength(), 0.0);
    ensure(empty->getEnvelopeInternal()->isNull());
    ensure(factory_->createEmpty(geos::geom::GEOS_SPIRALCURVE)->getGeometryTypeId()
           == geos::geom::GEOS_SPIRALCURVE);
    ensure_THROW(empty->getPointAtFraction(0.5), geos::util::IllegalArgumentException);
    for (SpiralType type : allTypes()) {
        ensure(SpiralCurve::typeFromName(SpiralCurve::typeName(type)) == type);
    }
    ensure(SpiralCurve::typeFromName("Bloss") == SpiralType::BLOSS);
    ensure_THROW(SpiralCurve::typeFromName("cubic"), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("clothoid type matches Clothoid");

    // Start at (10, 20) heading 0.3, curvature 0 -> 0.02 over 50.
    const double c = std::cos(0.3);
    const double s = std::sin(0.3);
    AffinePlacement at{Coordinate(10, 20, 0), Coordinate(c, s, 0), Coordinate(-s, c, 0)};
    auto spiral = make(50, 0, 0.02, SpiralType::CLOTHOID, at);
    auto clothoid = factory_->createClothoid(
                        CoordinateXYZM(10, 20, geos::DoubleNotANumber, geos::DoubleNotANumber),
                        false, false, 0.3, 0.0, 0.02, 50);
    ensure(spiral->getStartPoint()->getCoordinate()->equals2D(CoordinateXY(10, 20)));
    ensure_equals("end x", spiral->getEndPoint()->getX(), clothoid->getEndPoint()->getX(), 1e-12);
    ensure_equals("end y", spiral->getEndPoint()->getY(), clothoid->getEndPoint()->getY(), 1e-12);
    ensure_equals("length", spiral->getLength(), 50.0, 1e-12);

    auto a = spiral->getLinearized(CurveToLineParams::stepSizeDegrees(1));
    auto b = clothoid->getLinearized(CurveToLineParams::stepSizeDegrees(1));
    ensure(a->getEnvelopeInternal()->getMaxY() > 20);
    ensure_equals("envelope", spiral->getEnvelopeInternal()->getMaxX(),
                  clothoid->getEnvelopeInternal()->getMaxX(), 1e-12);
    ensure_equals("envelope", spiral->getEnvelopeInternal()->getMaxY(),
                  clothoid->getEnvelopeInternal()->getMaxY(), 1e-12);
    ensure(a->getNumPoints() > 2);
    ensure(b->getNumPoints() > 2);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("constant curvature is a circular arc for every type");

    const double k = 0.05;
    const double L = 40;
    for (SpiralType type : allTypes()) {
        auto arc = make(L, k, k, type);
        ensure_equals("x", arc->getEndPoint()->getX(), std::sin(k * L) / k, 1e-12);
        ensure_equals("y", arc->getEndPoint()->getY(), (1 - std::cos(k * L)) / k, 1e-12);
        ensure_equals("length", arc->getLength(), L, 1e-12);
        for (int i = 1; i < 10; ++i) {
            const CoordinateXYZM p = arc->getPointAtFraction(i / 10.0);
            ensure_equals("radius", std::hypot(p.x, p.y - 1 / k), 1 / k, 1e-12);
        }
    }
    auto line = make(25, 0, 0, SpiralType::SINE);
    ensure(line->getEndPoint()->getCoordinate()->equals2D(CoordinateXY(25, 0)));
    ensure(!line->hasCurvedComponents());
}

template<>
template<>
void object::test<4>()
{
    set_test_name("every type against an independent integration");

    for (SpiralType type : allTypes()) {
        for (auto [k0, k1] : {std::pair<double, double>{0.0, 0.01}, {0.004, -0.02}, {-0.01, 0.03}}) {
            auto spiral = make(120, k0, k1, type);
            for (double t : {0.25, 0.5, 0.8, 1.0}) {
                const CoordinateXY ref = reference(120, k0, k1, type, t);
                const CoordinateXYZM p = spiral->getPointAtFraction(t);
                const std::string at = SpiralCurve::typeName(type) + " t=" + std::to_string(t);
                ensure_equals(("x " + at).c_str(), p.x, ref.x, 1e-9);
                ensure_equals(("y " + at).c_str(), p.y, ref.y, 1e-9);
            }
            ensure_equals("length", spiral->getLength(), 120.0, 1e-11);
        }
    }
}

template<>
template<>
void object::test<5>()
{
    set_test_name("types share their ends' heading but differ in between");

    // Each shape integrates to 1/2, so every type ends at heading
    // L (k0 + k1) / 2; the reversed placement points the other way.
    const double L = 80;
    const double k0 = 0.005;
    const double k1 = 0.02;
    const double end = L * (k0 + k1) / 2;
    std::vector<CoordinateXYZM> mids;
    for (SpiralType type : allTypes()) {
        auto spiral = make(L, k0, k1, type);
        // Keep the reversed curve alive while reading its placement.
        auto reversed = spiral->reverse();
        const auto& u = reversed->getReferenceLocation().u;
        ensure_equals("heading x", u.x, -std::cos(end), 1e-14);
        ensure_equals("heading y", u.y, -std::sin(end), 1e-14);
        mids.push_back(spiral->getPointAtFraction(0.5));
    }
    for (std::size_t i = 1; i < mids.size(); ++i) {
        ensure(mids[i].distance(mids[0]) > 1e-6);
    }
}

template<>
template<>
void object::test<6>()
{
    set_test_name("reverse traces the same curve backwards");

    for (SpiralType type : allTypes()) {
        AffinePlacement at{Coordinate(5, -3, 0), Coordinate(0.6, 0.8, 0), Coordinate(-0.8, 0.6, 0)};
        auto spiral = make(60, -0.01, 0.025, type, at);
        auto back = spiral->reverse();
        ensure(back->getSpiralType() == type);
        ensure_equals(back->getStartCurvature(), -0.025);
        ensure_equals(back->getEndCurvature(), 0.01);
        ensure(back->getStartPoint()->getCoordinate()->equals2D(*spiral->getEndPoint()->getCoordinate()));
        for (int i = 0; i <= 10; ++i) {
            const CoordinateXYZM p = spiral->getPointAtFraction(i / 10.0);
            const CoordinateXYZM q = back->getPointAtFraction(1 - i / 10.0);
            ensure_equals("x", q.x, p.x, 1e-11);
            ensure_equals("y", q.y, p.y, 1e-11);
        }
        ensure_equals("length", back->getLength(), spiral->getLength(), 1e-11);
    }
}

template<>
template<>
void object::test<7>()
{
    set_test_name("envelope covers the curve tightly");

    // An S-curve: the curvature changes sign, so the heading has two
    // monotone pieces, on a sheared placement.
    AffinePlacement at{Coordinate(1, 2, 0), Coordinate(0.9, 0.2, 0), Coordinate(-0.4, 1.1, 0)};
    for (SpiralType type : allTypes()) {
        auto spiral = make(300, -0.03, 0.04, type, at);
        geos::geom::Envelope sampled;
        for (int i = 0; i <= 20000; ++i) {
            const CoordinateXYZM p = spiral->getPointAtFraction(i / 20000.0);
            ensure(spiral->getEnvelopeInternal()->covers(p.x, p.y));
            sampled.expandToInclude(p.x, p.y);
        }
        const auto* env = spiral->getEnvelopeInternal();
        ensure_equals("min x", env->getMinX(), sampled.getMinX(), 1e-4);
        ensure_equals("max x", env->getMaxX(), sampled.getMaxX(), 1e-4);
        ensure_equals("min y", env->getMinY(), sampled.getMinY(), 1e-4);
        ensure_equals("max y", env->getMaxY(), sampled.getMaxY(), 1e-4);
    }
}

template<>
template<>
void object::test<8>()
{
    set_test_name("invalid construction is rejected");

    using geos::util::IllegalArgumentException;
    ensure_THROW(make(0, 0, 0.01, SpiralType::BLOSS), IllegalArgumentException);
    ensure_THROW(make(-5, 0, 0.01, SpiralType::BLOSS), IllegalArgumentException);
    ensure_THROW(make(geos::DoubleNotANumber, 0, 0.01, SpiralType::BLOSS), IllegalArgumentException);
    ensure_THROW(make(10, geos::DoubleInfinity, 0.01, SpiralType::BLOSS), IllegalArgumentException);
    ensure_THROW(make(1e6, 0, 0.01, SpiralType::BLOSS), IllegalArgumentException);
    ensure_THROW(make(10, 0, 0.01, SpiralType::BLOSS,
                      AffinePlacement{Coordinate(0, geos::DoubleNotANumber, 0), Coordinate(1, 0, 0), Coordinate(0, 1, 0)}),
                 IllegalArgumentException);
    ensure_THROW(factory_->createSpiralCurve(identityAt(0, 0), 10, 0, 0.01, SpiralType::SINE, false, true,
                 0.0, geos::DoubleInfinity),
                 IllegalArgumentException);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("WKT round trip and form");

    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(17);
    const std::string zm =
        "SPIRALCURVE ZM (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION Z (1 2 3), "
        "REFERENCEDIRECTIONS (VECTOR Z (0 1 0), VECTOR Z (-1 0 0))), LENGTH 100, "
        "STARTCURVATURE 0, ENDCURVATURE 0.002, SPIRALTYPE bloss, STARTM 10, ENDM 20)";
    auto curve = reader_.read<SpiralCurve>(zm);
    ensure(curve->getSpiralType() == SpiralType::BLOSS);
    ensure_equals(writer.write(curve.get()), zm);
    ensure(curve->getStartPoint()->getCoordinate()->equals2D(CoordinateXY(1, 2)));
    ensure_equals(curve->getStartPoint()->getZ(), 3.0);
    ensure_equals(curve->getEndPoint()->getM(), 20.0);

    for (SpiralType type : allTypes()) {
        auto spiral = make(33.3, 0.001, -0.004, type, identityAt(7, 8));
        auto back = reader_.read<SpiralCurve>(writer.write(spiral.get()));
        ensure(spiral->equalsIdentical(back.get()));
    }
    ensure(reader_.read<SpiralCurve>(
               "SPIRALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
               "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), LENGTH 10, "
               "STARTCURVATURE 0, ENDCURVATURE 0.01, SPIRALTYPE COSINE)")->getSpiralType() == SpiralType::COSINE);
    ensure_equals(writer.write(reader_.read("SPIRALCURVE EMPTY").get()), std::string("SPIRALCURVE EMPTY"));
}

template<>
template<>
void object::test<10>()
{
    set_test_name("rejected WKT");

    using geos::io::ParseException;
    const std::string head =
        "(REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), LENGTH 10, "
        "STARTCURVATURE 0, ENDCURVATURE 0.01, SPIRALTYPE ";
    ensure(reader_.read("SPIRALCURVE " + head + "sine)") != nullptr);
    ensure_THROW(reader_.read("SPIRALCURVE " + head + "cubic)"), ParseException);
    ensure_THROW(reader_.read("SPIRALCURVE " + head + "sine, STARTM 0, ENDM 1)"), ParseException);
    ensure_THROW(reader_.read("SPIRALCURVE M " + head + "sine)"), ParseException);
    ensure_THROW(reader_.read("SPIRALCURVE Z " + head + "sine)"), ParseException);
    ensure_THROW(reader_.read("SPIRALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                              "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), LENGTH EMPTY, "
                              "STARTCURVATURE 0, ENDCURVATURE 0.01, SPIRALTYPE sine)"), ParseException);
    ensure_THROW(reader_.read("SPIRALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                              "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), LENGTH -1, "
                              "STARTCURVATURE 0, ENDCURVATURE 0.01, SPIRALTYPE sine)"),
                 geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<11>()
{
    set_test_name("WKB layout and round trips");

    // Identity placement at the origin, length 1, curvature 0 -> 0, bloss.
    const char* hexLiteral =
        "0117000000"
        "0166000000" "0000000000000000" "0000000000000000"
        "02000000" "000000000000F03F" "0000000000000000" "0000000000000000" "000000000000F03F"
        "000000000000F03F" "0000000000000000" "0000000000000000"
        "05" "626C6F7373";
    auto spiral = make(1, 0, 0, SpiralType::BLOSS);
    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*spiral, hex);
    ensure_equals(hex.str(), std::string(hexLiteral));
    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream hexIn(hexLiteral);
    ensure(spiral->equalsIdentical(wkbReader.readHEX(hexIn).get()));

    auto zm = reader_.read(
                  "SPIRALCURVE ZM (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION Z (1 2 3), "
                  "REFERENCEDIRECTIONS (VECTOR Z (0 1 0), VECTOR Z (-1 0 0))), LENGTH 100, "
                  "STARTCURVATURE 0, ENDCURVATURE 0.002, SPIRALTYPE biquadratic, STARTM 10, ENDM 20)");
    for (int flavor : {geos::io::WKBConstants::wkbExtended, geos::io::WKBConstants::wkbIso}) {
        for (int order : {geos::io::ByteOrderValues::ENDIAN_LITTLE, geos::io::ByteOrderValues::ENDIAN_BIG}) {
            geos::io::WKBWriter writer(4, order, false, flavor);
            std::stringstream out;
            writer.writeHEX(*zm, out);
            ensure(zm->equalsIdentical(wkbReader.readHEX(out).get()));
        }
    }

    std::stringstream badType(std::string(hexLiteral).substr(0, std::string(hexLiteral).size() - 12) + "05" + "6375626963");
    ensure_THROW(wkbReader.readHEX(badType), geos::io::ParseException);
}

template<>
template<>
void object::test<12>()
{
    set_test_name("WKB empty, alone and before another geometry");

    geos::io::WKBWriter wkbWriter;
    std::stringstream emptyHex;
    wkbWriter.writeHEX(*factory_->createSpiralCurve(false, false), emptyHex);
    ensure_equals(emptyHex.str(), std::string("0117000000"));

    geos::io::WKBReader wkbReader(*factory_);
    for (int order : {geos::io::ByteOrderValues::ENDIAN_LITTLE, geos::io::ByteOrderValues::ENDIAN_BIG}) {
        for (const char* wkt : {
                    "MULTICURVE (SPIRALCURVE EMPTY, (0 0, 1 1))",
                    "MULTICURVE (SPIRALCURVE EMPTY, CLOTHOID EMPTY)",
                    "MULTICURVE (SPIRALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                    "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), LENGTH 10, "
                    "STARTCURVATURE 0, ENDCURVATURE 0.01, SPIRALTYPE sine), SPIRALCURVE EMPTY)"
                }) {
            auto multi = reader_.read(wkt);
            geos::io::WKBWriter writer(2, order);
            std::stringstream hex;
            writer.writeHEX(*multi, hex);
            ensure(wkt, multi->equalsIdentical(wkbReader.readHEX(hex).get()));
        }
    }
}

template<>
template<>
void object::test<13>()
{
    set_test_name("linearization honors deviation and step");

    // Constant curvature, so the chord deviation is the circle's sagitta.
    const double k = 0.1;
    auto arc = make(20, k, k, SpiralType::COSINE);
    for (double deviation : {1e-1, 1e-3, 1e-6}) {
        auto line = arc->getLinearized(CurveToLineParams::maxDeviation(deviation));
        const auto* seq = line->getCoordinatesRO();
        ensure(seq->getAt<CoordinateXY>(0).equals2D(CoordinateXY(0, 0)));
        for (std::size_t i = 1; i < seq->size(); ++i) {
            const CoordinateXY& a = seq->getAt<CoordinateXY>(i - 1);
            const CoordinateXY& b = seq->getAt<CoordinateXY>(i);
            const double half = 0.5 * a.distance(b);
            ensure((1 / k) - std::sqrt(1 / (k * k) - half * half) <= deviation);
        }
    }
    // 2 rad of turn in 5 degree steps.
    auto stepped = arc->getLinearized(CurveToLineParams::stepSizeDegrees(5));
    ensure_equals(stepped->getNumPoints(), std::size_t(1 + 23));

    auto measured = reader_.read<SpiralCurve>(
                        "SPIRALCURVE M (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), LENGTH 10, "
                        "STARTCURVATURE 0, ENDCURVATURE 0, SPIRALTYPE clothoid, STARTM 0, ENDM 10)");
    auto linear = measured->getLinearized(CurveToLineParams());
    const auto* seq = linear->getCoordinatesRO();
    for (std::size_t i = 0; i < seq->size(); ++i) {
        ensure_equals("m", seq->getM(i), seq->getX(i), 1e-12);
    }
}

template<>
template<>
void object::test<14>()
{
    set_test_name("affine image, equality and normalize");

    const double c = std::cos(0.7);
    const double s = std::sin(0.7);
    auto spiral = make(50, 0.002, 0.03, SpiralType::SINE, identityAt(3, 4));
    spiral->setSRID(28992);
    auto image = spiral->affineTransform(c, -s, 100, s, c, -50);
    ensure_equals(image->getSRID(), 28992);
    for (int i = 0; i <= 10; ++i) {
        const CoordinateXYZM p = spiral->getPointAtFraction(i / 10.0);
        const CoordinateXYZM q = image->getPointAtFraction(i / 10.0);
        ensure_equals("x", q.x, c * p.x - s * p.y + 100, 1e-12);
        ensure_equals("y", q.y, s * p.x + c * p.y - 50, 1e-12);
    }
    ensure(spiral->equalsIdentical(spiral->clone().get()));
    ensure_equals(spiral->compareTo(spiral->clone().get()), 0);
    auto other = make(50, 0.002, 0.03, SpiralType::COSINE, identityAt(3, 4));
    ensure(!spiral->equalsExact(other.get()));
    ensure(spiral->compareTo(other.get()) != 0);

    auto backwards = make(10, 0, 0.01, SpiralType::BLOSS, AffinePlacement{
        Coordinate(10, 0, 0), Coordinate(-1, 0, 0), Coordinate(0, -1, 0)});
    auto normalized = backwards->clone();
    normalized->normalize();
    ensure(normalized->getStartPoint()->getCoordinate()->compareTo(
               *normalized->getEndPoint()->getCoordinate()) <= 0);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("CompoundCurve with a transition spiral");

    // Straight, then a clothoid transition into a curve.
    auto spiral = make(30, 0, 0.02, SpiralType::BLOSS, identityAt(10, 0));
    const CoordinateXYZM end = spiral->getPointAtFraction(1.0);
    std::vector<std::unique_ptr<SimpleCurve>> sections;
    sections.push_back(factory_->createLineString({CoordinateXY(0, 0), CoordinateXY(10, 0)}));
    sections.push_back(spiral->clone());
    sections.push_back(factory_->createLineString({CoordinateXY(end.x, end.y), CoordinateXY(end.x + 5, end.y + 5)}));
    auto compound = factory_->createCompoundCurve(std::move(sections));

    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(17);
    const std::string text = writer.write(compound.get());
    ensure(text.find("COMPOUNDCURVE ((0 0, 10 0), SPIRALCURVE (") == 0);
    ensure(compound->equalsIdentical(reader_.read(text).get()));
    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*compound, hex);
    geos::io::WKBReader wkbReader(*factory_);
    ensure(compound->equalsIdentical(wkbReader.readHEX(hex).get()));
    ensure(compound->getLinearized(CurveToLineParams::maxDeviation(1e-3))->getNumPoints() > 4);
    ensure_equals("length", compound->getLength(), 10 + 30 + std::sqrt(50.0), 1e-10);

    auto point = factory_->createPoint(CoordinateXY(0, 5));
    ensure_THROW(point->distance(compound.get()), geos::util::UnsupportedOperationException);
    auto line = factory_->createLineString({CoordinateXY(20, -5), CoordinateXY(20, 20)});
    ensure_THROW(spiral->intersection(line.get()), geos::util::UnsupportedOperationException);
    PointLocator locator;
    ensure_THROW(locator.locate(CoordinateXY(10, 0), spiral.get()), geos::util::UnsupportedOperationException);
}

template<>
template<>
void object::test<16>()
{
    set_test_name("clothoid I/O is unchanged by the shared placement code");

    const std::string text =
        "CLOTHOID Z (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION Z (1 2 3), "
        "REFERENCEDIRECTIONS (VECTOR Z (1 0 0), VECTOR Z (0 1 0))), SCALEFACTOR 50, "
        "STARTDISTANCE 0, ENDDISTANCE 25)";
    auto clothoid = reader_.read(text);
    geos::io::WKTWriter writer;
    ensure_equals(writer.write(clothoid.get()), text);
    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*clothoid, hex);
    geos::io::WKBReader wkbReader(*factory_);
    ensure(clothoid->equalsIdentical(wkbReader.readHEX(hex).get()));
}

} // namespace tut
