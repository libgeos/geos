//
// Test Suite for PreparedGeometry's touches() functions

// tut
#include <tut.hpp>
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

namespace tut {

//
// Test Group
//

struct test_preparedgeometrytouches_data
{
    typedef std::auto_ptr<geos::geom::prep::PreparedGeometry> PrepGeomAutoPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::GeometryFactory::unique_ptr factory;
    geos::io::WKTReader reader;
    GeometryPtr g1;
    GeometryPtr g2;
    PreparedGeometryPtr pg1;
    PreparedGeometryPtr pg2;

    test_preparedgeometrytouches_data()
        : factory(GeometryFactory::create())
        , reader(factory.get())
        , g1(0)
        , g2(0)
        , pg1(0)
        , pg2(0)
    {}
    ~test_preparedgeometrytouches_data()
    {
        prep::PreparedGeometryFactory::destroy(pg1);
        prep::PreparedGeometryFactory::destroy(pg2);
        factory->destroyGeometry(g1);
        factory->destroyGeometry(g2);
    }
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
void object::test<1>()
{
    g1 = reader.read(
        "POINT (0 0)"
        );
    g2 = reader.read(
        "POINT (0 0)"
        );
    pg1 = prep::PreparedGeometryFactory::prepare(g1);
    pg2 = prep::PreparedGeometryFactory::prepare(g2);

    ensure(!pg1->touches(g2));
    ensure(!pg2->touches(g1));
}

// 2 - Line/Point do not touch if point is not on boundary
template<>
template<>
void object::test<2>()
{
    g1 = reader.read(
        "LINESTRING(0 0, 1 1, 0 2)"
        );
    g2 = reader.read(
        "POINT (1 1)"
        );
    pg1 = prep::PreparedGeometryFactory::prepare(g1);
    pg2 = prep::PreparedGeometryFactory::prepare(g2);

    ensure(!pg1->touches(g2));
    ensure(!pg2->touches(g1));
}

// 3 - Line/Point touch
template<>
template<>
void object::test<3>()
{
    g1 = reader.read(
        "LINESTRING(0 0, 1 1, 0 2)"
        );
    g2 = reader.read(
        "POINT (0 2)"
        );
    pg1 = prep::PreparedGeometryFactory::prepare(g1);
    pg2 = prep::PreparedGeometryFactory::prepare(g2);

    ensure(pg1->touches(g2));
    ensure(pg2->touches(g1));
}

// 4 - Line/Point touch (FP coordinates)
template<>
template<>
void object::test<4>()
{
    g1 = reader.read(
        "LINESTRING (-612844.96290006 279079.117329031,-257704.820935236 574364.179187424)"
        );
    g2 = reader.read(
        "POINT (-257704.820935236 574364.179187424)"
        );
    pg1 = prep::PreparedGeometryFactory::prepare(g1);
    pg2 = prep::PreparedGeometryFactory::prepare(g2);

    ensure(pg1->touches(g2));
    ensure(pg2->touches(g1));
}

} // namespace tut
