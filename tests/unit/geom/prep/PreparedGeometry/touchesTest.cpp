//
// Test Suite for PreparedGeometry's touches() functions

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>

using namespace geos::geom;
using geos::geom::prep::PreparedGeometry;

namespace tut {

//
// Test Group
//

struct test_preparedgeometrytouches_data {
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::GeometryFactory::Ptr factory;
    geos::io::WKTReader reader;
    std::unique_ptr<geos::geom::Geometry> g1;
    std::unique_ptr<geos::geom::Geometry> g2;
    std::unique_ptr<PreparedGeometry> pg1;
    std::unique_ptr<PreparedGeometry> pg2;

    test_preparedgeometrytouches_data()
        : factory(GeometryFactory::create())
        , reader(factory.get())
        , g1(nullptr)
        , g2(nullptr)
        , pg1(nullptr)
        , pg2(nullptr)
    {}
};

typedef test_group<test_preparedgeometrytouches_data> group;
typedef group::object object;

group test_preparedgeometrytouches_data("geos::geom::prep::PreparedGeometry::touches");

//
// Test Cases
//

// 1 - Point/Point do not touch
template<>
template<>
void object::test<1>
()
{
    g1 = reader.read(
             "POINT (0 0)"
         );
    g2 = reader.read(
             "POINT (0 0)"
         );
    pg1 = prep::PreparedGeometryFactory::prepare(g1.get());
    pg2 = prep::PreparedGeometryFactory::prepare(g2.get());

    ensure(!pg1->touches(g2.get()));
    ensure(!pg2->touches(g1.get()));
}

// 2 - Line/Point do not touch if point is not on boundary
template<>
template<>
void object::test<2>
()
{
    g1 = reader.read(
             "LINESTRING(0 0, 1 1, 0 2)"
         );
    g2 = reader.read(
             "POINT (1 1)"
         );
    pg1 = prep::PreparedGeometryFactory::prepare(g1.get());
    pg2 = prep::PreparedGeometryFactory::prepare(g2.get());

    ensure(!pg1->touches(g2.get()));
    ensure(!pg2->touches(g1.get()));
}

// 3 - Line/Point touch
template<>
template<>
void object::test<3>
()
{
    g1 = reader.read(
             "LINESTRING(0 0, 1 1, 0 2)"
         );
    g2 = reader.read(
             "POINT (0 2)"
         );
    pg1 = prep::PreparedGeometryFactory::prepare(g1.get());
    pg2 = prep::PreparedGeometryFactory::prepare(g2.get());

    ensure(pg1->touches(g2.get()));
    ensure(pg2->touches(g1.get()));
}

// 4 - Line/Point touch (FP coordinates)
template<>
template<>
void object::test<4>
()
{
    g1 = reader.read(
             "LINESTRING (-612844.96290006 279079.117329031,-257704.820935236 574364.179187424)"
         );
    g2 = reader.read(
             "POINT (-257704.820935236 574364.179187424)"
         );
    pg1 = prep::PreparedGeometryFactory::prepare(g1.get());
    pg2 = prep::PreparedGeometryFactory::prepare(g2.get());

    ensure(pg1->touches(g2.get()));
    ensure(pg2->touches(g1.get()));
}

} // namespace tut
