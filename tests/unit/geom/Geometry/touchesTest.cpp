//
// Test Suite for Geometry's touches() functions

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
// std
#include <sstream>
#include <memory>

namespace tut {

//
// Test Group
//

struct test_touches_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::GeometryFactory::Ptr factory;
    geos::io::WKTReader reader;
    geos::io::WKBReader breader;

    test_touches_data()
        : factory(GeometryFactory::create())
        , reader(factory.get())
        , breader(*factory.get())
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
void object::test<1>
()
{
    GeomPtr g1(reader.read(
                   "POINT (0 0)"
               ));

    GeomPtr g2(reader.read(
                   "POINT (0 0)"
               ));

    ensure(!g1->touches(g2.get()));
    ensure(!g2->touches(g1.get()));
}

// 2 - Line/Point do not touch if point is not on boundary
template<>
template<>
void object::test<2>
()
{
    GeomPtr g1(reader.read(
                   "LINESTRING(0 0, 1 1, 0 2)"
               ));

    GeomPtr g2(reader.read(
                   "POINT (1 1)"
               ));

    ensure(!g1->touches(g2.get()));
    ensure(!g2->touches(g1.get()));
}

// 3 - Line/Point touch
template<>
template<>
void object::test<3>
()
{
    GeomPtr g1(reader.read(
                   "LINESTRING(0 0, 1 1, 0 2)"
               ));

    GeomPtr g2(reader.read(
                   "POINT (0 2)"
               ));

    ensure(g1->touches(g2.get()));
    ensure(g2->touches(g1.get()));
}

// 4 - Line/Point touch (FP coordinates)
template<>
template<>
void object::test<4>
()
{
    GeomPtr g1(reader.read(
                   "LINESTRING (-612844.96290006 279079.117329031,-257704.820935236 574364.179187424)"
               ));

    GeomPtr g2(reader.read(
                   "POINT (-257704.820935236 574364.179187424)"
               ));

    ensure(g1->touches(g2.get()));
    ensure(g2->touches(g1.get()));
}

template<>
template<>
void object::test<5>
()
{
    // Two T-like segments, A (horizontal), B (vertical)
    // A: LINESTRING(-3511.75501903694 4257.47493284327,-877.546556856658 4257.47493284327)
    // B: LINESTRING(-2119.81532027122 4257.47493284327,-2119.81532027122 2326.7198668134)
    std::stringstream wkbA("01020000000200000010efda91826fabc0a8e5329579a1b040008633595f6c8bc0a8e5329579a1b040");
    std::stringstream wkbB("0102000000020000005999a871a18fa0c0a8e5329579a1b0405999a871a18fa0c0180a6292702da240");
    GeomPtr a(breader.readHEX(wkbA));
    GeomPtr b(breader.readHEX(wkbB));

    ensure(a->touches(b.get()));
    ensure(!a->disjoint(b.get()));
    ensure(a->intersects(b.get()));
}

template<>
template<>
void object::test<6>
()
{
    // Two Y-like segments, A (V-part), B (|-part)
    // A: LINESTRING(-428.533750803201 4467.01424233489,1098.10978977856 4137.73818456235,1621.95806350759 5544.64497686319)
    // B: LINESTRING(1098.10978977856 4137.73818456235,1921.2999342099 2177.04893146225)
    std::stringstream
    wkbA("010200000003000000603f483e8ac87ac092ba62a50373b1405851bb6c70289140b6d9a9f9bc29b04060a2990ed55799401226341da5a8b540");
    std::stringstream wkbB("0102000000020000005851bb6c70289140b6d9a9f9bc29b040d019f42133059e40406c8b0d1902a140");
    GeomPtr a(breader.readHEX(wkbA));
    GeomPtr b(breader.readHEX(wkbB));

    ensure(a->touches(b.get()));
    ensure(!a->disjoint(b.get()));
    ensure(a->intersects(b.get()));
}

template<>
template<>
void object::test<7>
()
{
    // Two T-like two segments rotated ~55 degrees counter-clockwise; A (horizontal), B (vertical)
    // A: LINESTRING(3343.17382004585 2521.2920827699,4959.61992183829 5125.56635787996)
    // B: LINESTRING(4151.39687094207 3823.42922032493,6112.08612404217 2461.42370862944)
    std::stringstream wkbA("01020000000200000098e8f0fe581eaa40ea70df8b95b2a3408c9532b39e5fb340417cd4fc9005b440");
    std::stringstream wkbB("010200000002000000ec8455996537b040b834c4c2dbdead4086a8390c16e0b740f86456f0d83aa340");
    GeomPtr a(breader.readHEX(wkbA));
    GeomPtr b(breader.readHEX(wkbB));

    // segments do not just touch, but intersect (float-point robustness issue likely)
    ensure(!a->touches(b.get()));
    ensure(!a->disjoint(b.get()));
    ensure(a->intersects(b.get()));
}

} // namespace tut
