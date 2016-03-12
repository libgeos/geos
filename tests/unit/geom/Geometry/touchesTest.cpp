//
// Test Suite for Geometry's touches() functions

// tut
#include <tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>

namespace tut {

//
// Test Group
//

struct test_touches_data
{
    typedef std::auto_ptr<geos::geom::Geometry> GeomAutoPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::GeometryFactory::unique_ptr factory;
    geos::io::WKTReader reader;

    test_touches_data()
        : factory(GeometryFactory::create())
        , reader(factory.get())
    {}
};

typedef test_group<test_touches_data> group;
typedef group::object object;

group test_touches_data("geos::geom::Geometry::touches");

//
// Test Cases
//

// 1 - Point/Point do not touch
template<>
template<>
void object::test<1>()
{
    GeomAutoPtr g1(reader.read(
        "POINT (0 0)"
        ));

    GeomAutoPtr g2(reader.read(
        "POINT (0 0)"
        ));

    ensure(!g1->touches(g2.get()));
    ensure(!g2->touches(g1.get()));
}

// 2 - Line/Point do not touch if point is not on boundary
template<>
template<>
void object::test<2>()
{
    GeomAutoPtr g1(reader.read(
        "LINESTRING(0 0, 1 1, 0 2)"
        ));

    GeomAutoPtr g2(reader.read(
        "POINT (1 1)"
        ));

    ensure(!g1->touches(g2.get()));
    ensure(!g2->touches(g1.get()));
}

// 3 - Line/Point touch
template<>
template<>
void object::test<3>()
{
    GeomAutoPtr g1(reader.read(
        "LINESTRING(0 0, 1 1, 0 2)"
        ));

    GeomAutoPtr g2(reader.read(
        "POINT (0 2)"
        ));

    ensure(g1->touches(g2.get()));
    ensure(g2->touches(g1.get()));
}

// 4 - Line/Point touch (FP coordinates)
template<>
template<>
void object::test<4>()
{
    GeomAutoPtr g1(reader.read(
        "LINESTRING (-612844.96290006 279079.117329031,-257704.820935236 574364.179187424)"
        ));

    GeomAutoPtr g2(reader.read(
        "POINT (-257704.820935236 574364.179187424)"
        ));

    ensure(g1->touches(g2.get()));
    ensure(g2->touches(g1.get()));
}

} // namespace tut
