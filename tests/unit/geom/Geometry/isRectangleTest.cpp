//
// Test Suite for Geometry::isRectangle() function

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
// std
#include <string>

namespace tut {
//
// Test Group
//

struct test_isrectangle_data {
    geos::io::WKTReader reader;

    test_isrectangle_data()
    {}
};

typedef test_group<test_isrectangle_data> group;
typedef group::object object;

group test_isrectangle_group("geos::geom::Geometry::isRectangle");

//
// Test Cases
//

// 1 - Test of valid rectangle
template<>
template<>
void object::test<1>
()
{
    const std::string wkt("POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))");
    auto g = reader.read(wkt);

    geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(g.get());
    ensure("Geometry is not a Polygon: " + wkt, poly != nullptr);
    ensure(poly->isRectangle());
}

// 2 - Test of another valid rectangle
template<>
template<>
void object::test<2>
()
{
    const std::string wkt("POLYGON ((0 0, 0 200, 100 200, 100 0, 0 0))");
    auto g = reader.read(wkt);

    geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(g.get());
    ensure("Geometry is not a Polygon: " + wkt, poly != nullptr);
    ensure(poly->isRectangle());
}

// 3 - Test of rectangle with hole
template<>
template<>
void object::test<3>
()
{
    const std::string wkt("POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0), \
                              (10 10, 10 90, 90 90, 90 10, 10 10) ))");
    auto g = reader.read(wkt);

    geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(g.get());
    ensure("Geometry is not a Polygon: " + wkt, poly != nullptr);
    ensure(!poly->isRectangle());
}

// 4 - Test of non-rectilinear rectangle
template<>
template<>
void object::test<4>
()
{
    const std::string wkt("POLYGON ((0 0, 0 100, 99 100, 100 0, 0 0))");
    auto g = reader.read(wkt);

    geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(g.get());
    ensure("Geometry is not a Polygon: " + wkt, poly != nullptr);
    ensure(!poly->isRectangle());
}

// 5 - Test of rectangle with too many points
template<>
template<>
void object::test<5>
()
{
    const std::string wkt("POLYGON ((0 0, 0 100, 100 50, 100 100, 100 0, 0 0))");
    auto g = reader.read(wkt);

    geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(g.get());
    ensure("Geometry is not a Polygon: " + wkt, poly != nullptr);
    ensure(!poly->isRectangle());
}

// 6 - Test of rectangle with too few points
template<>
template<>
void object::test<6>
()
{
    const std::string wkt("POLYGON ((0 0, 0 100, 100 0, 0 0))");
    auto g = reader.read(wkt);

    geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(g.get());
    ensure("Geometry is not a Polygon: " + wkt, poly != nullptr);
    ensure(!poly->isRectangle());
}

// 7 - Test of rectangle with points in wrong order
template<>
template<>
void object::test<7>
()
{
    const std::string wkt("POLYGON ((0 0, 0 100, 100 0, 100 100, 0 0))");
    auto g = reader.read(wkt);

    geos::geom::Polygon* poly = dynamic_cast<geos::geom::Polygon*>(g.get());
    ensure("Geometry is not a Polygon: " + wkt, poly != nullptr);
    ensure(!poly->isRectangle());
}

} // namespace tut

