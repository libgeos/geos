//
// Test Suite for geos::operation::valid::IsValidOp class
// Ported from JTS junit/operation/valid/IsValidTest.java rev. 1.1

#include <tut/tut.hpp>
// geos
#include <geos/constants.h> // for std::isnan
#include <geos/operation/valid/IsValidOp.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/operation/valid/TopologyValidationError.h>
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
void object::test<1>
()
{
    CoordinateArraySequence* cs = new CoordinateArraySequence();
    cs->add(Coordinate(0.0, 0.0));
    cs->add(Coordinate(1.0, geos::DoubleNotANumber));
    GeomPtr line(factory_->createLineString(cs));


    IsValidOp isValidOp(line.get());
    bool valid = isValidOp.isValid();

    TopologyValidationError* err = isValidOp.getValidationError();
    ensure(nullptr != err);
    const Coordinate& errCoord = err->getCoordinate();

    ensure_equals(err->getErrorType(),
                  TopologyValidationError::eInvalidCoordinate);

    ensure(0 != std::isnan(errCoord.y));
    ensure_equals(valid, false);
}

template<>
template<>
void object::test<2>
()
{
    std::string wkt0("POLYGON((25495445.625 6671632.625,25495445.625 6671711.375,25495555.375 6671711.375,25495555.375 6671632.625,25495445.625 6671632.625),(25495368.0441 6671726.9312,25495368.3959388 6671726.93601515,25495368.7478 6671726.9333,25495368.0441 6671726.9312))");
    GeomPtr g0(wktreader.read(wkt0));

    IsValidOp isValidOp(g0.get());
    bool valid = isValidOp.isValid();

    TopologyValidationError* err = isValidOp.getValidationError();
    ensure(nullptr != err);
    const Coordinate& errCoord = err->getCoordinate();

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
void object::test<3>
()
{
    // https://trac.osgeo.org/geos/ticket/588

    std::string wkt("POLYGON (( -86.3958130146539250 114.3482370100377900, 64.7285128575111490 156.9678884302379600, 138.3490775437400700 43.1639042523018260, 87.9271046586986810 -10.5302909001479570, 87.9271046586986810 -10.5302909001479530, 55.7321237336437390 -44.8146215164960250, -86.3958130146539250 114.3482370100377900))");
    auto g = wktreader.read(wkt);

    ensure(g->isValid());

    auto g_rev = g->reverse();

    ensure(g_rev->isValid());
}

} // namespace tut
