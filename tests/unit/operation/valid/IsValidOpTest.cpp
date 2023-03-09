//
// Test Suite for geos::operation::valid::IsValidOp class
// Ported from JTS junit/operation/valid/IsValidTest.java rev. 1.1

#include <tut/tut.hpp>
// geos
#include <geos/constants.h> // for std::isnan
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/valid/TopologyValidationError.h>
#include <geos/util.h>
// std
#include <cmath>
#include <string>
#include <memory>

using namespace geos::geom;
using namespace geos::operation::valid;

namespace tut {
//
// Test Group
//

struct test_isvalidop_data {
    geos::io::WKTReader wktreader;
    typedef std::unique_ptr<Geometry> GeomPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::PrecisionModel pm_;
    GeometryFactory::Ptr factory_;

    test_isvalidop_data()
        : pm_(1), factory_(GeometryFactory::create(&pm_, 0))
    {}

    void checkValid(const char* wkt)
    {
        std::string wktstr(wkt);
        auto g = wktreader.read(wktstr);
        ensure(g->isValid());
    }

    void checkInvalid(const char* wkt)
    {
        std::string wktstr(wkt);
        auto g = wktreader.read(wktstr);
        ensure(!g->isValid());
    }

    void checkInvalid(int errExpected, const char* wkt)
    {
        std::string wktstr(wkt);
        auto geom = wktreader.read(wktstr);
        IsValidOp validOp(geom.get());
        int err = validOp.getValidationError()->getErrorType();
        ensure_equals("error codes do not match", err, errExpected);
    }

};

typedef test_group<test_isvalidop_data> group;
typedef group::object object;

group test_isvalidop_group("geos::operation::valid::IsValidOp");

//
// Test Cases
//

// 1 - testInvalidCoordinate
template<>
template<>
void object::test<1> ()
{
    auto cs = geos::detail::make_unique<CoordinateSequence>();
    cs->add(Coordinate(0.0, 0.0));
    cs->add(Coordinate(1.0, geos::DoubleNotANumber));
    auto line = factory_->createLineString(std::move(cs));


    IsValidOp isValidOp(line.get());
    bool valid = isValidOp.isValid();

    const TopologyValidationError* err = isValidOp.getValidationError();
    ensure(nullptr != err);
    const auto& errCoord = err->getCoordinate();

    ensure_equals(err->getErrorType(),
                  TopologyValidationError::eInvalidCoordinate);

    ensure(0 != std::isnan(errCoord.y));
    ensure_equals(valid, false);
}

template<>
template<>
void object::test<29> ()
{
    auto cs = geos::detail::make_unique<CoordinateSequence>();
    cs->add(Coordinate(0.0, 0.0));
    cs->add(Coordinate(1.0, geos::DoubleInfinity));
    auto line = factory_->createLineString(std::move(cs));


    IsValidOp isValidOp(line.get());
    bool valid = isValidOp.isValid();

    const TopologyValidationError* err = isValidOp.getValidationError();
    ensure(nullptr != err);
    const auto& errCoord = err->getCoordinate();

    ensure_equals(err->getErrorType(),
                  TopologyValidationError::eInvalidCoordinate);

    ensure(!std::isfinite(errCoord.y));
    ensure_equals(valid, false);
}

template<>
template<>
void object::test<2> ()
{
    std::string wkt0("POLYGON((25495445.625 6671632.625,25495445.625 6671711.375,25495555.375 6671711.375,25495555.375 6671632.625,25495445.625 6671632.625),(25495368.0441 6671726.9312,25495368.3959388 6671726.93601515,25495368.7478 6671726.9333,25495368.0441 6671726.9312))");
    GeomPtr g0(wktreader.read(wkt0));

    IsValidOp isValidOp(g0.get());
    bool valid = isValidOp.isValid();

    const TopologyValidationError* err = isValidOp.getValidationError();
    ensure(nullptr != err);
    const auto& errCoord = err->getCoordinate();

    ensure_equals(err->getErrorType(),
                  TopologyValidationError::eHoleOutsideShell);

    ensure(0 == std::isnan(errCoord.y));
    ensure(0 == std::isnan(errCoord.x));
    ensure(fabs(errCoord.y - 6671726.9) < 1.0);
    ensure(fabs(errCoord.x - 25495368.0) < 1.0);
    ensure_equals(valid, false);
}

template<>
template<>
void object::test<3> ()
{
    // https://trac.osgeo.org/geos/ticket/588

    std::string wkt("POLYGON (( -86.3958130146539250 114.3482370100377900, 64.7285128575111490 156.9678884302379600, 138.3490775437400700 43.1639042523018260, 87.9271046586986810 -10.5302909001479570, 87.9271046586986810 -10.5302909001479530, 55.7321237336437390 -44.8146215164960250, -86.3958130146539250 114.3482370100377900))");
    auto g = wktreader.read(wkt);

    ensure(g->isValid());

    auto g_rev = g->reverse();

    ensure(g_rev->isValid());
}

template<>
template<>
void object::test<4> ()
{
    // https://github.com/locationtech/jts/pull/737

    std::string wkt("LINEARRING (150 100, 300 300, 100 300, 350 100, 150 100)");
    auto g = wktreader.read(wkt);

    ensure(! g->isValid());
}

template<>
template<>
void object::test<5> ()
{
    std::string wkt("MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2)),((60 60, 60 50, 70 40, 60 60)))");
    auto g = wktreader.read(wkt);
    ensure(g->isValid());
}

template<>
template<>
void object::test<6> ()
{
    std::string wkt("POLYGON((40 320,340 320,340 20,40 20,40 320),(100 120,40 20,180 100,100 120),(200 200,180 100,240 160,200 200),(260 260,240 160,300 200,260 260),(300 300,300 200,340 260,300 300))");
    auto g = wktreader.read(wkt);
    ensure(!g->isValid());
}

// testValidSimplePolygon
template<>
template<>
void object::test<7> ()
{
    checkValid(
        "POLYGON ((10 89, 90 89, 90 10, 10 10, 10 89))");
}

// testInvalidSimplePolygonRingSelfIntersection
template<>
template<>
void object::test<8> ()
{
    checkInvalid(
        TopologyValidationError::eSelfIntersection,
        "POLYGON ((10 90, 90 10, 90 90, 10 10, 10 90))");
}

// testInvalidPolygonInverted
template<>
template<>
void object::test<22> ()
{
    checkInvalid(
        TopologyValidationError::eRingSelfIntersection,
        "POLYGON ((70 250, 40 500, 100 400, 70 250, 80 350, 60 350, 70 250))");
}

// testSimplePolygonHole
template<>
template<>
void object::test<9> ()
{
    checkValid(
        "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (60 20, 20 70, 90 90, 60 20))");
}

// testPolygonTouchingHoleAtVertex
template<>
template<>
void object::test<10> ()
{
    checkValid(
        "POLYGON ((240 260, 40 260, 40 80, 240 80, 240 260), (140 180, 40 260, 140 240, 140 180))");
}

// testInvalidPolygonHoleProperIntersection
template<>
template<>
void object::test<11> ()
{
    checkInvalid(
        TopologyValidationError::eSelfIntersection,
        "POLYGON ((10 90, 50 50, 10 10, 10 90), (20 50, 60 70, 60 30, 20 50))");
}

// testInvalidPolygonDisconnectedInterior
template<>
template<>
void object::test<12> ()
{
    checkInvalid(
        TopologyValidationError::eDisconnectedInterior,
        "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 30 80, 20 20, 20 80), (80 30, 20 20, 80 20, 80 30), (80 80, 30 80, 80 30, 80 80))");
}

// testValidMultiPolygonTouchAtVertices
template<>
template<>
void object::test<13> ()
{
    checkValid(
        "MULTIPOLYGON (((10 10, 10 90, 90 90, 90 10, 80 80, 50 20, 20 80, 10 10)), ((90 10, 10 10, 50 20, 90 10)))");
}

// testValidMultiPolygonTouchAtVerticesSegments
template<>
template<>
void object::test<14> ()
{
    checkValid(
        "MULTIPOLYGON (((60 40, 90 10, 90 90, 10 90, 10 10, 40 40, 60 40)), ((50 40, 20 20, 80 20, 50 40)))");
}

// testInvalidMultiPolygonNestedAllTouchAtVertices
template<>
template<>
void object::test<15> ()
{
    checkInvalid(
        TopologyValidationError::eNestedShells,
        "MULTIPOLYGON (((10 10, 20 30, 10 90, 90 90, 80 30, 90 10, 50 20, 10 10)), ((80 30, 20 30, 50 20, 80 30)))");
}

// testValidMultiPolygonHoleTouchVertices
template<>
template<>
void object::test<16> ()
{
    checkValid(
        "MULTIPOLYGON (((20 380, 420 380, 420 20, 20 20, 20 380), (220 340, 80 320, 60 200, 140 100, 340 60, 300 240, 220 340)), ((60 200, 340 60, 220 340, 60 200)))");
}

// testPolygonMultipleHolesTouchAtSamePoint
template<>
template<>
void object::test<17> ()
{
    checkValid(
        "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (40 80, 60 80, 50 50, 40 80), (20 60, 20 40, 50 50, 20 60), (40 20, 60 20, 50 50, 40 20))");
}

// testPolygonHoleOutsideShellAllTouch
template<>
template<>
void object::test<18> ()
{
    checkInvalid(TopologyValidationError::eHoleOutsideShell,
        "POLYGON ((10 10, 30 10, 30 50, 70 50, 70 10, 90 10, 90 90, 10 90, 10 10), (50 50, 30 10, 70 10, 50 50))");
}

// testPolygonHoleOutsideShellDoubleTouch
template<>
template<>
void object::test<19> ()
{
    checkInvalid(TopologyValidationError::eHoleOutsideShell,
        "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 80 80, 80 20, 20 20, 20 80), (90 70, 150 50, 90 20, 110 40, 90 70))");
}

// testPolygonNestedHolesAllTouch
template<>
template<>
void object::test<20> ()
{
    checkInvalid(TopologyValidationError::eNestedHoles,
        "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 80 80, 80 20, 20 20, 20 80), (50 80, 80 50, 50 20, 20 50, 50 80))");
}

// testInvalidMultiPolygonHoleOverlapCrossing
template<>
template<>
void object::test<21> ()
{
    checkInvalid(
        TopologyValidationError::eSelfIntersection,
        "MULTIPOLYGON (((20 380, 420 380, 420 20, 20 20, 20 380), (220 340, 180 240, 60 200, 140 100, 340 60, 300 240, 220 340)), ((60 200, 340 60, 220 340, 60 200)))");
}

// testLineString
template<>
template<>
void object::test<23> ()
{
    checkInvalid(
        "LINESTRING(0 0, 0 0)");
}

// testLinearRingTriangle
template<>
template<>
void object::test<24> ()
{
    checkValid(
        "LINEARRING (100 100, 150 200, 200 100, 100 100)");
}

// testLinearRingSelfCrossing
template<>
template<>
void object::test<25> ()
{
    checkInvalid(TopologyValidationError::eRingSelfIntersection,
        "LINEARRING (150 100, 300 300, 100 300, 350 100, 150 100)");
}

// testLinearRingSelfCrossing2
template<>
template<>
void object::test<26> ()
{
    checkInvalid(TopologyValidationError::eRingSelfIntersection,
        "LINEARRING (0 0, 100 100, 100 0, 0 100, 0 0)");
}

//
template<>
template<>
void object::test<27> ()
{
    checkInvalid(TopologyValidationError::eRingSelfIntersection,
        "POLYGON ((70 250, 40 500, 100 400, 70 250, 80 350, 60 350, 70 250))");
}

template<>
template<>
void object::test<28> ()
{
    checkInvalid(TopologyValidationError::eSelfIntersection,
        "POLYGON ((70 250, 70 500, 80 400, 40 400, 70 250))");
}


} // namespace tut
