//
// Test Suite for geos::operation::IsSimpleOp class
// Ported from JTS junit/operation/IsSimpleTest.java

#include <tut/tut.hpp>
// geos
#include <geos/operation/IsSimpleOp.h>
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

group test_issimpleop_group("geos::operation::IsSimpleOp");

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

    IsSimpleOp op;
    bool simple = op.isSimpleLinearGeometry(geom.get());

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

    IsSimpleOp op;
    bool simple = op.isSimpleLinearGeometry(geom.get());

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

    IsSimpleOp op;
    bool simple = op.isSimpleLinearGeometry(geom.get());

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

} // namespace tut
