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
#include <geos/io/ParseException.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using geos::algorithm::CurveToLineParams;
using geos::algorithm::PointLocator;
using geos::geom::Clothoid;
using geos::geom::ClothoidReferenceLocation;
using geos::geom::CompoundCurve;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYZM;
using geos::geom::SimpleCurve;

namespace tut {

struct test_clothoid_data {
    geos::geom::GeometryFactory::Ptr factory_ = geos::geom::GeometryFactory::create();
    geos::io::WKTReader reader_;

    std::unique_ptr<Clothoid> make(
        double x, double y, double theta0, double k0, double k1, double length) const
    {
        return factory_->createClothoid(
                   CoordinateXYZM(x, y, geos::DoubleNotANumber, geos::DoubleNotANumber),
                   false, false, theta0, k0, k1, length);
    }

    static ClothoidReferenceLocation identityAt(double x, double y, double z = 0.0)
    {
        return ClothoidReferenceLocation{
            Coordinate(x, y, z),
            Coordinate(1.0, 0.0, 0.0),
            Coordinate(0.0, 1.0, 0.0)};
    }

    std::unique_ptr<CompoundCurve> lineThenClothoid() const
    {
        std::vector<std::unique_ptr<SimpleCurve>> sections;
        sections.push_back(factory_->createLineString({
            CoordinateXY(0.0, 0.0), CoordinateXY(1.0, 0.0)}));
        sections.push_back(make(1.0, 0.0, 0.0, 0.0, 0.01, 5.0));
        return factory_->createCompoundCurve(std::move(sections));
    }
};

typedef test_group<test_clothoid_data> group;
typedef group::object object;

group test_clothoid_group("geos::geom::Clothoid");

template<>
template<>
void object::test<1>()
{
    set_test_name("type id");

    ensure_equals(static_cast<int>(geos::geom::GEOS_CLOTHOID), 13);
    auto empty = factory_->createClothoid(false, false);
    ensure_equals(empty->getGeometryTypeId(), geos::geom::GEOS_CLOTHOID);
    ensure_equals(empty->getGeometryType(), "Clothoid");
}

template<>
template<>
void object::test<2>()
{
    set_test_name("empty");

    auto empty = factory_->createClothoid(false, false);
    ensure(empty->isEmpty());
    ensure_equals(empty->getLength(), 0.0);
    ensure_equals(empty->getNumPoints(), 0u);
    ensure(empty->hasCurvedTypes());
    ensure(!empty->hasCurvedComponents());
    ensure(empty->getEnvelopeInternal()->isNull());

    auto parsed = reader_.read("CLOTHOID EMPTY");
    ensure(parsed->isEmpty());
    ensure_equals(parsed->getGeometryTypeId(), geos::geom::GEOS_CLOTHOID);

    auto zempty = reader_.read("CLOTHOID Z EMPTY");
    ensure(zempty->hasZ());
    ensure(!zempty->hasM());
    auto mempty = reader_.read("CLOTHOID M EMPTY");
    ensure(mempty->hasM());
    ensure(!mempty->hasZ());
}

template<>
template<>
void object::test<3>()
{
    set_test_name("WKT round trip");

    auto curve = make(1.0, 2.0, 0.25, 0.0, 0.125, 8.0);
    geos::io::WKTWriter writer;
    writer.setRoundingPrecision(15);
    const std::string text = writer.write(curve.get());
    ensure(text.find("REFERENCELOCATION") != std::string::npos);
    ensure(text.find("CLOTHOID (1") == std::string::npos);

    auto back = reader_.read<Clothoid>(text);
    ensure(curve->equalsExact(back.get(), 1e-9));
    ensure_equals("heading", back->getStartHeading(), curve->getStartHeading(), 1e-9);
    ensure_equals("k0", back->getStartCurvature(), 0.0, 1e-9);
    ensure_equals("k1", back->getEndCurvature(), 0.125, 1e-9);
    ensure_equals("length", back->getLength(), curve->getLength(), 1e-9);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("WKB Z");

    auto curve = factory_->createClothoid(
                     CoordinateXYZM(1.5, -2.5, 9.0, geos::DoubleNotANumber),
                     true, false, 0.3, -0.01, 0.02, 4.0);
    geos::io::WKBWriter writer;
    std::stringstream hex;
    writer.writeHEX(*curve, hex);
    geos::io::WKBReader wkbReader(*factory_);
    auto back = wkbReader.readHEX(hex);
    ensure(curve->equalsIdentical(back.get()));
    const auto* backClothoid = dynamic_cast<const Clothoid*>(back.get());
    ensure(backClothoid != nullptr);
    ensure_equals("z0", backClothoid->getCoordinatesRO()->getZ(0), 9.0, 0.0);
    ensure_equals("z1", backClothoid->getCoordinatesRO()->getZ(1), 9.0, 0.0);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("length");

    auto curve = reader_.read<Clothoid>(
                     "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                     "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), "
                     "SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 25)");
    ensure_equals("length", curve->getLength(), 25.0, 0.0);
    ensure_equals("x", curve->getEndCoordinate().x, 24.960965746830126, 1e-9);
    ensure_equals("y", curve->getEndCoordinate().y, 1.0405046700886817, 1e-9);
    ensure_equals("heading", curve->getEndHeading(), 0.125, 1e-12);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("envelope");

    auto turning = make(0.0, 0.0, 0.0, 0.0, 2.0, geos::MATH_PI);
    auto line = turning->getLinearized(CurveToLineParams::maxDeviation(1e-4));
    const geos::geom::Envelope* env = turning->getEnvelopeInternal();
    const auto* seq = line->getCoordinatesRO();
    for (std::size_t i = 0; i < seq->size(); ++i) {
        const CoordinateXY pt = seq->getAt<CoordinateXY>(i);
        ensure(env->covers(pt.x, pt.y));
    }
    ensure(env->getMaxX() > turning->getEndCoordinate().x + 0.05);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("getLinearized()");

    auto curve = make(0.0, 0.0, 0.2, 0.1, -0.05, 3.0);
    auto line = curve->getLinearized(CurveToLineParams::maxDeviation(1e-4));
    ensure_equals(line->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure(line->getNumPoints() >= 3u);
    ensure_equals("x0", line->getCoordinatesRO()->front<CoordinateXY>().x, curve->getStartCoordinate().x, 1e-12);
    ensure_equals("x1", line->getCoordinatesRO()->back<CoordinateXY>().x, curve->getEndCoordinate().x, 1e-9);
    ensure_equals("y1", line->getCoordinatesRO()->back<CoordinateXY>().y, curve->getEndCoordinate().y, 1e-9);
}

template<>
template<>
void object::test<8>()
{
    set_test_name("getLinearized() Z/M");

    auto curve = reader_.read<Clothoid>(
                     "CLOTHOID ZM (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION Z (1 2 3), "
                     "REFERENCEDIRECTIONS (VECTOR Z (1 0 0), VECTOR Z (0 1 0))), "
                     "SCALEFACTOR 8, STARTDISTANCE -2, ENDDISTANCE 2, STARTM 1, ENDM 5)");
    auto line = curve->getLinearized(CurveToLineParams::maxDeviation(0.1));
    const auto* seq = line->getCoordinatesRO();
    ensure_equals("z0", seq->getZ(0), 3.0, 1e-12);
    ensure_equals("z1", seq->getZ(seq->size() - 1), 3.0, 1e-12);
    ensure_equals("m0", seq->getM(0), 1.0, 1e-12);
    ensure_equals("m1", seq->getM(seq->size() - 1), 5.0, 1e-12);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("reverse");

    auto curve = make(0.0, 0.0, 0.2, 0.1, -0.05, 3.0);
    auto reversed = curve->reverse();
    ensure_equals("x", reversed->getStartCoordinate().x, curve->getEndCoordinate().x, 1e-9);
    ensure_equals("y", reversed->getStartCoordinate().y, curve->getEndCoordinate().y, 1e-9);
    ensure_equals("k0", reversed->getStartCurvature(), -curve->getEndCurvature(), 1e-12);
    ensure_equals("k1", reversed->getEndCurvature(), -curve->getStartCurvature(), 1e-12);
    ensure_equals("length", reversed->getLength(), curve->getLength(), 1e-9);

    auto restored = reversed->reverse();
    ensure_equals("heading", restored->getStartHeading(), curve->getStartHeading(), 1e-12);
    ensure_equals("scale", restored->getScaleFactor(), curve->getScaleFactor(), 0.0);
    ensure_equals("t0", restored->getStartDistance(), curve->getStartDistance(), 0.0);
    ensure_equals("t1", restored->getEndDistance(), curve->getEndDistance(), 0.0);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("measures");

    auto curve = factory_->createClothoid(
                     CoordinateXYZM(0.0, 0.0, geos::DoubleNotANumber, 2.0),
                     false, true, 0.0, 0.0, 0.01, 10.0, 7.0);
    ensure(curve->hasM());
    ensure_equals("startM", curve->getStartM(), 2.0, 0.0);
    ensure_equals("endM", curve->getEndM(), 7.0, 0.0);
    ensure_equals("m0", curve->getCoordinatesRO()->getM(0), 2.0, 0.0);
    ensure_equals("m1", curve->getCoordinatesRO()->getM(1), 7.0, 0.0);
}

template<>
template<>
void object::test<11>()
{
    set_test_name("constant curvature");

    ensure_THROW(make(0.0, 0.0, 0.0, 0.0, 0.0, 2.0), geos::util::IllegalArgumentException);
    ensure_THROW(make(0.0, 0.0, 0.0, 0.01, 0.01, 2.0), geos::util::IllegalArgumentException);
    ensure_THROW(make(0.0, 0.0, 0.0, 0.0, 0.01, -1.0), geos::util::IllegalArgumentException);
    ensure_THROW(
        make(0.0, 0.0, 0.0, 0.0, 0.01, std::numeric_limits<double>::infinity()),
        geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<12>()
{
    set_test_name("gentle curve");

    auto gentle = factory_->createClothoid(identityAt(0.0, 0.0), 1.0e6, 0.0, 2.0, false, false, 0.0, 0.0);
    ensure(gentle->hasCurvedComponents());
    ensure_equals("length", gentle->getLength(), 2.0, 1e-9);
    ensure_equals("x", gentle->getEndCoordinate().x, 2.0, 1e-6);
    ensure(std::abs(gentle->getEndCoordinate().y) < 1e-6);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("large turn");

    ensure_THROW(make(0.0, 0.0, 0.0, 0.0, 1e5, 20000.0), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("large curvature");

    ensure_THROW(make(0.0, 0.0, 0.0, 0.0, 1.0, 1e308), geos::util::IllegalArgumentException);
    ensure_THROW(make(0.0, 0.0, 0.0, 0.0, 1e308, 1.0), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("compound linearize");

    auto compound = lineThenClothoid();
    const CoordinateXY end = compound->getCurveN(1)->getEndCoordinate();
    auto line = compound->getLinearized(CurveToLineParams::maxDeviation(0.05));
    ensure(line->getNumPoints() > 3u);
    ensure_equals("x0", line->getCoordinatesRO()->front<CoordinateXY>().x, 0.0, 1e-12);
    ensure_equals("x1", line->getCoordinatesRO()->back<CoordinateXY>().x, end.x, 1e-8);
    ensure_equals("y1", line->getCoordinatesRO()->back<CoordinateXY>().y, end.y, 1e-8);
}

template<>
template<>
void object::test<16>()
{
    set_test_name("distance");

    auto compound = lineThenClothoid();
    auto point = factory_->createPoint(CoordinateXY(0.0, 10.0));
    ensure_THROW(point->distance(compound.get()), geos::util::UnsupportedOperationException);
}

template<>
template<>
void object::test<17>()
{
    set_test_name("intersection");

    auto compound = lineThenClothoid();
    auto line = factory_->createLineString({
        CoordinateXY(0.0, 0.0), CoordinateXY(1.0, 0.0)});
    ensure_THROW(compound->intersection(line.get()), geos::util::UnsupportedOperationException);
}

template<>
template<>
void object::test<18>()
{
    set_test_name("point location");

    auto compound = lineThenClothoid();
    const CoordinateXY end = compound->getCurveN(1)->getEndCoordinate();
    PointLocator locator;
    const geos::geom::Geometry* geom = compound.get();
    ensure_THROW(locator.locate(end, geom), geos::util::UnsupportedOperationException);
}

template<>
template<>
void object::test<19>()
{
    set_test_name("WKB hex");

    const char* wkt =
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
        "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), "
        "SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 25)";
    auto curve = reader_.read<Clothoid>(wkt);
    ensure_equals("scale", curve->getScaleFactor(), 50.0, 0.0);
    ensure_equals("ux", curve->getReferenceLocation().u.x, 1.0, 0.0);
    ensure_equals("vy", curve->getReferenceLocation().v.y, 1.0, 0.0);

    const char* hexLiteral =
        "0116000000"
        "0166000000"
        "0000000000000000"
        "0000000000000000"
        "02000000"
        "000000000000F03F"
        "0000000000000000"
        "0000000000000000"
        "000000000000F03F"
        "0000000000004940"
        "0000000000000000"
        "0000000000003940";
    geos::io::WKBWriter wkbWriter;
    std::stringstream hex;
    wkbWriter.writeHEX(*curve, hex);
    ensure_equals(hex.str(), std::string(hexLiteral));

    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream hexIn(hexLiteral);
    auto fromHex = wkbReader.readHEX(hexIn);
    ensure(curve->equalsIdentical(fromHex.get()));
}

template<>
template<>
void object::test<20>()
{
    set_test_name("WKB empty");

    geos::io::WKBWriter wkbWriter;
    std::stringstream emptyHex;
    wkbWriter.writeHEX(*factory_->createClothoid(false, false), emptyHex);
    ensure_equals(emptyHex.str(), std::string("0116000000"));
    std::stringstream emptyZ;
    wkbWriter.writeHEX(*factory_->createClothoid(true, false), emptyZ);
    ensure_equals(emptyZ.str(), std::string("0116000080"));
    std::stringstream emptyM;
    wkbWriter.writeHEX(*factory_->createClothoid(false, true), emptyM);
    ensure_equals(emptyM.str(), std::string("0116000040"));
    std::stringstream emptyZM;
    wkbWriter.writeHEX(*factory_->createClothoid(true, true), emptyZM);
    ensure_equals(emptyZM.str(), std::string("01160000C0"));

    geos::io::WKBWriter isoWriter(
        4, geos::io::ByteOrderValues::ENDIAN_LITTLE, false, geos::io::WKBConstants::wkbIso);
    std::stringstream isoZ;
    isoWriter.writeHEX(*factory_->createClothoid(true, false), isoZ);
    ensure_equals(isoZ.str(), std::string("01FE030000"));
    std::stringstream isoM;
    isoWriter.writeHEX(*factory_->createClothoid(false, true), isoM);
    ensure_equals(isoM.str(), std::string("01E6070000"));
    std::stringstream isoZM;
    isoWriter.writeHEX(*factory_->createClothoid(true, true), isoZM);
    ensure_equals(isoZM.str(), std::string("01CE0B0000"));
}

template<>
template<>
void object::test<21>()
{
    set_test_name("rejected WKT");

    ensure_THROW(reader_.read("CLOTHOID (0 0, 0, 0, 0.01, 25)"), geos::io::ParseException);
    ensure_THROW(reader_.read(
                     "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                     "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), "
                     "SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 25, STARTM 1, ENDM 2)"),
                 geos::io::ParseException);
    ensure_THROW(reader_.read(
                     "CLOTHOID M (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                     "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), "
                     "SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 25)"),
                 geos::io::ParseException);
}

template<>
template<>
void object::test<22>()
{
    set_test_name("WKB type 18");

    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream circle("0112000000");
    ensure_THROW(wkbReader.readHEX(circle), geos::io::ParseException);
    std::stringstream circleZ("01FA030000");
    ensure_THROW(wkbReader.readHEX(circleZ), geos::io::ParseException);
}

template<>
template<>
void object::test<23>()
{
    set_test_name("WKB type 23");

    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream spiral("0117000000");
    ensure_THROW(wkbReader.readHEX(spiral), geos::io::ParseException);
}

template<>
template<>
void object::test<24>()
{
    set_test_name("sheared placement");

    ClothoidReferenceLocation sheared{
        Coordinate(0.0, 0.0, 0.0),
        Coordinate(2.0, 0.0, 0.0),
        Coordinate(0.0, 1.0, 0.0)};
    auto curve = factory_->createClothoid(sheared, 1.0, 0.0, 1.0, false, false, 0.0, 0.0);
    ensure_equals("x", curve->getEndCoordinate().x, 1.9505753764006892, 1e-9);
    ensure_equals("y", curve->getEndCoordinate().y, 0.16371404737570058, 1e-9);
}

template<>
template<>
void object::test<25>()
{
    set_test_name("inflection");

    auto through = factory_->createClothoid(identityAt(0.0, 0.0), 10.0, -5.0, 5.0, false, false, 0.0, 0.0);
    ensure_equals("length", through->getLength(), 10.0, 1e-12);
    ensure_equals("x", through->getStartCoordinate().x, -through->getEndCoordinate().x, 1e-9);
    ensure_equals("y", through->getStartCoordinate().y, -through->getEndCoordinate().y, 1e-9);
    ensure(through->getEnvelopeInternal()->covers(0.0, 0.0));

    auto zero = factory_->createClothoid(identityAt(3.0, 4.0), 10.0, 1.0, 1.0, false, false, 0.0, 0.0);
    ensure_equals("length", zero->getLength(), 0.0, 0.0);
    ensure_equals("x", zero->getStartCoordinate().x, zero->getEndCoordinate().x, 0.0);
}

template<>
template<>
void object::test<26>()
{
    set_test_name("decreasing curvature");

    auto easing = make(0.0, 0.0, 0.0, 0.02, 0.01, 50.0);
    ensure_equals("k0", easing->getStartCurvature(), 0.02, 1e-12);
    ensure_equals("k1", easing->getEndCurvature(), 0.01, 1e-12);
    ensure_equals("x0", easing->getStartCoordinate().x, 0.0, 1e-12);
    ensure_equals("y0", easing->getStartCoordinate().y, 0.0, 1e-12);
    ensure_equals("heading", easing->getStartHeading(), 0.0, 1e-12);
    ensure(easing->getEndCoordinate().x > 40.0);
    ensure(easing->getEndCoordinate().y > 10.0);

    auto turned = make(3.0, 4.0, 1.0, -0.002, -0.03, 40.0);
    ensure_equals("x0", turned->getStartCoordinate().x, 3.0, 1e-12);
    ensure_equals("y0", turned->getStartCoordinate().y, 4.0, 1e-12);
    ensure_equals("k0", turned->getStartCurvature(), -0.002, 1e-12);
    ensure_equals("k1", turned->getEndCurvature(), -0.03, 1e-12);
}

template<>
template<>
void object::test<27>()
{
    set_test_name("Z WKT");

    auto curve = reader_.read<Clothoid>(
                     "CLOTHOID Z (REFERENCELOCATION AFFINEPLACEMENT Z (LOCATION Z (1 2 9), "
                     "REFERENCEDIRECTIONS (VECTOR Z (1 0 0), VECTOR Z (0 1 0))), "
                     "SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 25)");
    ensure(curve->hasZ());
    ensure(!curve->hasM());
    ensure_equals("z", curve->getCoordinatesRO()->getZ(0), 9.0, 0.0);
    geos::io::WKTWriter writer;
    auto back = reader_.read<Clothoid>(writer.write(curve.get()));
    ensure(curve->equalsIdentical(back.get()));
}

template<>
template<>
void object::test<28>()
{
    set_test_name("M WKT");

    auto curve = reader_.read<Clothoid>(
                     "CLOTHOID M (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), "
                     "REFERENCEDIRECTIONS (VECTOR (1 0), VECTOR (0 1))), "
                     "SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 25, STARTM 4, ENDM 9)");
    ensure_equals("startM", curve->getStartM(), 4.0, 0.0);
    ensure_equals("endM", curve->getEndM(), 9.0, 0.0);
    geos::io::WKTWriter writer;
    auto back = reader_.read<Clothoid>(writer.write(curve.get()));
    ensure(curve->equalsIdentical(back.get()));
}

template<>
template<>
void object::test<29>()
{
    set_test_name("bad placement");

    geos::io::WKBReader wkbReader(*factory_);
    std::stringstream hex(
        "0116000000"
        "0000000000000000"
        "0000000000000000"
        "0000000000000000"
        "0000000000000000"
        "0000000000000000"
        "0000000000000000");
    ensure_THROW(wkbReader.readHEX(hex), geos::io::ParseException);
}

template<>
template<>
void object::test<30>()
{
    set_test_name("ISO clothoid after a line");

    // The evaluated start of this clothoid is one ulp short of (1 0).
    const std::string clothoid =
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (-3.999166699734872 0.0833273811026917), "
        "REFERENCEDIRECTIONS (VECTOR (0.9996875162757026 -0.0249973959147123), "
        "VECTOR (0.0249973959147123 0.9996875162757026))), "
        "SCALEFACTOR 22.360679774997898, STARTDISTANCE 5, ENDDISTANCE 10)";
    const CoordinateXY junction(1.0, 0.0);
    auto alone = reader_.read<Clothoid>(clothoid);
    ensure(alone->getStartCoordinate() != junction);
    ensure(alone->getStartCoordinate().distance(junction) < 1e-15);

    auto curve = reader_.read<CompoundCurve>("COMPOUNDCURVE ((0 0, 1 0), " + clothoid + ")");
    ensure(curve->getCurveN(1)->getStartCoordinate() == junction);

    geos::io::WKTWriter writer;
    auto fromWkt = reader_.read<CompoundCurve>(writer.write(curve.get()));
    ensure(fromWkt->getCurveN(1)->getStartCoordinate() == junction);

    geos::io::WKBWriter wkbWriter;
    std::stringstream wkb;
    wkbWriter.write(*curve, wkb);
    geos::io::WKBReader wkbReader(*factory_);
    auto fromWkb = wkbReader.read(wkb);
    const auto* compound = dynamic_cast<const CompoundCurve*>(fromWkb.get());
    ensure(compound != nullptr);
    ensure(compound->getCurveN(1)->getStartCoordinate() == junction);

    std::vector<std::unique_ptr<SimpleCurve>> sections;
    sections.push_back(factory_->createLineString({CoordinateXY(0.0, 0.0), junction}));
    sections.push_back(make(1.0, 0.0, 0.0, 0.01, 0.02, 5.0));
    auto built = factory_->createCompoundCurve(std::move(sections));
    ensure(built->getCurveN(1)->getStartCoordinate() == junction);
    auto reversed = built->reverse();
    ensure(reversed->getCurveN(0)->getEndCoordinate() == junction);
    reversed->normalize();
    ensure(reversed->getCurveN(1)->getStartCoordinate() == junction);
}

template<>
template<>
void object::test<31>()
{
    set_test_name("ISO clothoid that misses the previous end");

    const std::string clothoid =
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (-3.999166699734872 0.0833273811026917), "
        "REFERENCEDIRECTIONS (VECTOR (0.9996875162757026 -0.0249973959147123), "
        "VECTOR (0.0249973959147123 0.9996875162757026))), "
        "SCALEFACTOR 22.360679774997898, STARTDISTANCE 5, ENDDISTANCE 10)";
    ensure_THROW(reader_.read("COMPOUNDCURVE ((0 0, 1.000000001 0), " + clothoid + ")"),
                 geos::util::IllegalArgumentException);
    ensure_THROW(reader_.read("COMPOUNDCURVE ((0 0, 1.000000000001 0), " + clothoid + ")"),
                 geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<32>()
{
    set_test_name("similarity test is relative at any scale");

    // The same shapes at plan scale 1 and at plan scale k: curvature scales
    // by 1/k and length by k.
    const double scales[] = {1e-13, 1e-6, 1e6};
    const double squash[] = {1.0, 0.1};
    for (double q : squash) {
        ClothoidReferenceLocation unit{
            Coordinate(0.0, 0.0, 0.0), Coordinate(1.0, 0.0, 0.0), Coordinate(0.0, q, 0.0)};
        auto ref = factory_->createClothoid(unit, 1.0, 0.5, 2.0, false, false, 0.0, 0.0);
        for (double k : scales) {
            ClothoidReferenceLocation small{
                Coordinate(0.0, 0.0, 0.0), Coordinate(k, 0.0, 0.0), Coordinate(0.0, q * k, 0.0)};
            auto curve = factory_->createClothoid(small, 1.0, 0.5, 2.0, false, false, 0.0, 0.0);
            ensure_equals("start curvature", curve->getStartCurvature() * k, ref->getStartCurvature(),
                          1e-12 * std::abs(ref->getStartCurvature()));
            ensure_equals("end curvature", curve->getEndCurvature() * k, ref->getEndCurvature(),
                          1e-12 * std::abs(ref->getEndCurvature()));
            ensure_equals("length", curve->getLength() / k, ref->getLength(), 1e-12 * ref->getLength());
        }
    }
}

} // namespace tut
