//
// Test Suite for geos::operation::valid::IsSimpleOp class


#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/valid/IsSimpleOp.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
#include <geos/util/GEOSException.h>

// std
#include <string>
#include <sstream>
#include <memory>

using geos::operation::valid::IsSimpleOp;
using geos::geom::Coordinate;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::algorithm::BoundaryNodeRule;

namespace tut {
//
// Test Group
//

struct test_issimpleop_data {

    geos::geom::PrecisionModel pm_;
    GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;
    double tolerance_;

    test_issimpleop_data()
        : pm_()
        , factory_(GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get())
        , tolerance_(0.00005)
    {}

    void checkIsSimple(
        std::string& wkt,
        const BoundaryNodeRule& bnRule,
        bool expectedResult)
    {
        checkIsSimple(wkt, bnRule, expectedResult, Coordinate::getNull());
    }

    void checkIsSimple(
        std::string& wkt,
        const BoundaryNodeRule& bnRule,
        bool expectedResult,
        Coordinate expectedLocation)
    {
        auto g = reader_.read(wkt);
        IsSimpleOp op(*g, bnRule);
        bool isSimple = op.isSimple();
        const auto& nonSimpleLoc = op.getNonSimpleLocation();
        // if geom is not simple, should have a valid location
        ensure("unexpected result", expectedResult == isSimple);
        ensure("not simple implies a non-simple location", isSimple || ! nonSimpleLoc.isNull());
        if (!isSimple && !nonSimpleLoc.isNull() && !expectedLocation.isNull()) {
            ensure(expectedLocation.distance(nonSimpleLoc) < tolerance_);
        }
    }

    void checkIsSimpleAll(
        std::string& wkt,
        const BoundaryNodeRule& bnRule,
        std::string& wktExpectedPts)
    {
        auto g = reader_.read(wkt);
        IsSimpleOp op(*g, bnRule);
        op.setFindAllLocations(true);
        op.isSimple();

        auto nonSimpleCoords = op.getNonSimpleLocations();
        std::unique_ptr<Geometry> nsPts(g->getFactory()->createMultiPoint(nonSimpleCoords));
        // std::cout << *nsPts << std::endl;
        auto expectedPts = reader_.read(wktExpectedPts);
        ensure_equals_geometry(expectedPts.get(), nsPts.get());
    }


};

typedef test_group<test_issimpleop_data> group;
typedef group::object object;

group test_issimpleop_group("geos::operation::valid::IsSimpleOp");

//
// Test Cases
//


template<>
template<>
void object::test<1>
()
{
    // Adapted from https://trac.osgeo.org/geos/ticket/858
    constexpr char kData[] =
            "00000000020000000e0000000000000000"
            "0000000000000000240424242424242424"
            "24242424280000000000ffffffffffff3b"
            "ffffffffffffffffffffffff4000010800"
            "0000030000003b01980000000000000000"
            "0000000000000000000000000000002900"
            "000000000100000000490001f34e537437"
            "6c6f63616c653500000000000000000000"
            "2800000000000000000000000000000000"
            "fb0000000000010700000000003a000000"
            "f100000000000000000000f60000000000"
            "0000000000000000000000000000000000"
            "0000000000000000200000000000000000"
            "0000000000000000000000000000000000";

    geos::io::WKBReader reader;
    std::istringstream s(kData);

    auto g = reader.readHEX(s);

    try {
        g->isSimple();
    } catch (geos::util::GEOSException &) {
        // no memory leaks or invalid reads on exception
    }
}


// test2TouchAtEndpoint
template<>
template<>
void object::test<2> ()
{
    std::string a("MULTILINESTRING((0 1, 1 1, 2 1), (0 0, 1 0, 2 1))");
    checkIsSimple(a, BoundaryNodeRule::getBoundaryRuleMod2(), true, Coordinate(2, 1));
    checkIsSimple(a, BoundaryNodeRule::getBoundaryEndPoint(), true, Coordinate(2, 1));
}

// test3TouchAtEndpoint
template<>
template<>
void object::test<3> ()
{
    // rings are simple under all rules
    std::string a("MULTILINESTRING ((0 1, 1 1, 2 1),   (0 0, 1 0, 2 1),  (0 2, 1 2, 2 1))");
    checkIsSimple(a, BoundaryNodeRule::getBoundaryRuleMod2(), true, Coordinate(2, 1));
    checkIsSimple(a, BoundaryNodeRule::getBoundaryEndPoint(), true, Coordinate(2, 1));
}

// testCross
template<>
template<>
void object::test<4> ()
{
    std::string a("MULTILINESTRING ((20 120, 120 20), (20 20, 120 120))");
    checkIsSimple(a, BoundaryNodeRule::getBoundaryRuleMod2(), false, Coordinate(70, 70));
    checkIsSimple(a, BoundaryNodeRule::getBoundaryEndPoint(), false, Coordinate(70, 70));
}

// testMultiLineStringWithRingTouchAtEndpoint
template<>
template<>
void object::test<5> ()
{
    std::string a("MULTILINESTRING ((100 100, 20 20, 200 20, 100 100), (100 200, 100 100))");
    // under Mod-2, the ring has no boundary, so the line intersects the interior ==> not simple
    checkIsSimple(a, BoundaryNodeRule::getBoundaryRuleMod2(), false, Coordinate(100, 100));
    // under Endpoint, the ring has a boundary point, so the line does NOT intersect the interior ==> simple
    checkIsSimple(a, BoundaryNodeRule::getBoundaryEndPoint(), true);
}

// testRing
template<>
template<>
void object::test<6> ()
{
    // rings are simple under all rules
    std::string a("LINESTRING (100 100, 20 20, 200 20, 100 100)");
    checkIsSimple(a, BoundaryNodeRule::getBoundaryRuleMod2(), true);
    checkIsSimple(a, BoundaryNodeRule::getBoundaryEndPoint(), true);
}


// testLinesAll
template<>
template<>
void object::test<7> ()
{
    // rings are simple under all rules
    std::string a("MULTILINESTRING ((10 20, 90 20), (10 30, 90 30), (50 40, 50 10))");
    std::string b("MULTIPOINT((50 20), (50 30))");
    checkIsSimpleAll(a, BoundaryNodeRule::getBoundaryRuleMod2(), b);
}

// testPolygonAll
template<>
template<>
void object::test<8> ()
{
    std::string a("POLYGON ((0 0, 7 0, 6 -1, 6 -0.1, 6 0.1, 3 5.9, 3 6.1, 3.1 6, 2.9 6, 0 0))");
    std::string b("MULTIPOINT((6 0), (3 6))");
    checkIsSimpleAll(a, BoundaryNodeRule::getBoundaryRuleMod2(), b);
}

// testMultiPointAll
template<>
template<>
void object::test<9> ()
{
    std::string a("MULTIPOINT((1 1), (1 2), (1 2), (1 3), (1 4), (1 4), (1 5), (1 5))");
    std::string b("MULTIPOINT((1 2), (1 4), (1 5))");
    checkIsSimpleAll(a, BoundaryNodeRule::getBoundaryRuleMod2(), b);
}

// testGeometryCollectionAll
template<>
template<>
void object::test<10> ()
{
    std::string a("GEOMETRYCOLLECTION(MULTILINESTRING ((10 20, 90 20), (10 30, 90 30), (50 40, 50 10)), MULTIPOINT((1 1), (1 2), (1 2), (1 3), (1 4), (1 4), (1 5), (1 5)))");
    std::string b("MULTIPOINT((50 20), (50 30), (1 2), (1 4), (1 5))");
    checkIsSimpleAll(a, BoundaryNodeRule::getBoundaryRuleMod2(), b);
}


} // namespace tut
