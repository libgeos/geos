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

using namespace geos::geom;
using namespace geos::operation;
using namespace geos::operation::valid;

namespace tut {
//
// Test Group
//

struct test_issimpleop_data {
    typedef geos::geom::GeometryFactory GeometryFactory;
    geos::geom::PrecisionModel pm_;
    GeometryFactory::Ptr factory_;
    geos::io::WKTReader reader_;
    double tolerance_;

    test_issimpleop_data()
        : pm_(1)
        , factory_(GeometryFactory::create(&pm_, 0))
        , reader_(factory_.get())
        , tolerance_(0.00005)
    {}
};

typedef test_group<test_issimpleop_data> group;
typedef group::object object;

group test_issimpleop_group("geos::operation::valid::IsSimpleOp");

//
// Test Cases
//

// 1 - Test cross
template<>
template<>
void object::test<1>
()
{
    const std::string wkt("MULTILINESTRING ((20 120, 120 20), (20 20, 120 120))");
    const Geometry::Ptr geom(reader_.read(wkt));

    // TODO - mloskot: What about support of new features of BoundaryNodeRule, in JTS

    IsSimpleOp op(*geom);
    bool simple = op.isSimple();

    ensure(false == simple);

    // TODO - mloskot:
    // There are missing features not (re)implemented in IsSimpleOp, in GEOS.
    // So, all tests in this suite have been simplified in comparison to original JTS tests.
    //
    //Coordinate loc(70, 70);
    //Coordinate nonSimpleLoc = op.getNonSimpleLocation();
    //loc.distance(nonSimpleLoc) < TOLERANCE
}

// 2 - Test MultiLineString with ring touching at the end point
template<>
template<>
void object::test<2>
()
{
    const std::string wkt("MULTILINESTRING ((100 100, 20 20, 200 20, 100 100), (100 200, 100 100))");
    const Geometry::Ptr geom(reader_.read(wkt));

    IsSimpleOp op(*geom);
    bool simple = op.isSimple();

    ensure(false == simple);
}

// 3 - Test simple LineString
template<>
template<>
void object::test<3>
()
{
    const std::string wkt("LINESTRING (100 100, 20 20, 200 20, 100 100)");
    const Geometry::Ptr geom(reader_.read(wkt));

    IsSimpleOp op(*geom);
    bool simple = op.isSimple();

    ensure(true == simple);
}


template<>
template<>
void object::test<4>
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

 // public void testLinesAll() {
 //    checkIsSimpleAll("MULTILINESTRING ((10 20, 90 20), (10 30, 90 30), (50 40, 50 10))",
 //        BoundaryNodeRule.MOD2_BOUNDARY_RULE,
 //        "");
 //  }

 // private void checkIsSimpleAll(String wkt, BoundaryNodeRule bnRule,
 //      String wktExpectedPts)
 //  {
 //    Geometry g = read(wkt);
 //    IsSimpleOp op = new IsSimpleOp(g, bnRule);
 //    op.setFindAllLocations(true);
 //    op.isSimple();
 //    List<Coordinate> nonSimpleCoords = op.getNonSimpleLocations();
 //    Geometry nsPts = g.getFactory().createMultiPointFromCoords(CoordinateArrays.toCoordinateArray(nonSimpleCoords));

 //    Geometry expectedPts = read(wktExpectedPts);
 //    checkEqual(expectedPts, nsPts);
 //  }

template<>
template<>
void object::test<5>
()
{
    const std::string wkt("MULTILINESTRING ((10 20, 90 20), (10 30, 90 30), (50 40, 50 10))");
    const Geometry::Ptr geom(reader_.read(wkt));

    const std::string wktNonSimple("MULTIPOINT((50 20), (50 30))");
    const Geometry::Ptr geomExpectedNonSimple(reader_.read(wktNonSimple));

    IsSimpleOp op(*geom);
    op.setFindAllLocations(true);
    bool simple = op.isSimple();
    const std::vector<Coordinate>& nonSimpleCoords = op.getNonSimpleLocations();
    Geometry::Ptr nsPts(geom->getFactory()->createMultiPoint(nonSimpleCoords));

    ensure(false == simple);
    ensure_equals_geometry(nsPts.get(), geomExpectedNonSimple.get());
}


} // namespace tut
