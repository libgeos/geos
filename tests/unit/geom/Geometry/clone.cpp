//
// Test Suite for Geometry's clone()

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <string>

namespace tut {

//
// Test Group
//

struct test_geometry_clone_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;
    GeometryFactory::Ptr factory;
    geos::io::WKTReader reader;

    test_geometry_clone_data()
        : factory(GeometryFactory::create())
        , reader(factory.get())
    {}
};

typedef test_group<test_geometry_clone_data> group;
typedef group::object object;

group test_geometry_clone_data("geos::geom::Geometry::clone");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    GeomPtr g1(reader.read(
                   "POINT (0 100)"
               ));
    g1->setSRID(66);
    GeomPtr g2(g1->clone());

    ensure(g1->equalsExact(g2.get()));
    ensure_equals(g1->getSRID(), 66);
    ensure_equals(g1->getSRID(), g2->getSRID());

}

template<>
template<>
void object::test<2>
()
{
    GeomPtr g1(reader.read(
                   "LINESTRING (0 0, 0 100, 100 100, 100 0)"
               ));
    g1->setSRID(66);
    GeomPtr g2(g1->clone());

    ensure(g1->equalsExact(g2.get()));
    ensure_equals(g1->getSRID(), 66);
    ensure_equals(g1->getSRID(), g2->getSRID());

}

template<>
template<>
void object::test<3>
()
{
    GeomPtr g1(reader.read(
                   "POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))"
               ));
    g1->setSRID(66);
    GeomPtr g2(g1->clone());

    ensure(g1->equalsExact(g2.get()));
    ensure_equals(g1->getSRID(), 66);
    ensure_equals(g1->getSRID(), g2->getSRID());

}

template<>
template<>
void object::test<4>
()
{
    GeomPtr g1(reader.read(
                   "MULTIPOINT (0 100, 5 6)"
               ));
    g1->setSRID(66);
    GeomPtr g2(g1->clone());

    ensure(g1->equalsExact(g2.get()));
    ensure_equals(g1->getSRID(), 66);
    ensure_equals(g1->getSRID(), g2->getSRID());

}

template<>
template<>
void object::test<5>
()
{
    GeomPtr g1(reader.read(
                   "MULTILINESTRING ((0 0, 0 100, 100 100, 100 0), (15 25, 25 52))"
               ));
    g1->setSRID(66);
    GeomPtr g2(g1->clone());

    ensure(g1->equalsExact(g2.get()));
    ensure_equals(g1->getSRID(), 66);
    ensure_equals(g1->getSRID(), g2->getSRID());

}

template<>
template<>
void object::test<6>
()
{
    GeomPtr g1(reader.read(
                   "MULTIPOLYGON (((0 0, 0 100, 100 100, 100 0, 0 0)))"
               ));
    g1->setSRID(66);
    GeomPtr g2(g1->clone());
    ensure(g1->equalsExact(g2.get()));
    ensure_equals(g1->getSRID(), 66);
    ensure_equals(g1->getSRID(), g2->getSRID());
}

template<>
template<>
void object::test<7>
()
{
    GeomPtr g1(reader.read(
                   "GEOMETRYCOLLECTION(MULTIPOLYGON (((0 0, 0 100, 100 100, 100 0, 0 0))),POINT(3 4))"
               ));
    g1->setSRID(66);
    GeomPtr g2(g1->clone());
    ensure(g1->equalsExact(g2.get()));
    ensure_equals(g1->getSRID(), 66);
    ensure_equals(g1->getSRID(), g2->getSRID());
}


} // namespace tut

