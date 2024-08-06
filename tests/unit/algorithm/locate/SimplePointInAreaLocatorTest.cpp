#include <tut/tut.hpp>

#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/geom/Location.h>
#include <geos/io/WKTReader.h>

using geos::geom::CoordinateXY;
using geos::geom::Location;

namespace tut {

struct test_simplepointinarealocator_data {
    geos::io::WKTReader reader;

    static void checkLocation(const geos::geom::Geometry&g,
                              const geos::geom::CoordinateXY& pt,
                              geos::geom::Location loc) {
        geos::algorithm::locate::SimplePointInAreaLocator locator(g);
        ensure_equals(locator.locate(&pt), loc);
    }

};

typedef test_group<test_simplepointinarealocator_data> group;
typedef group::object object;

group test_simplepointinarealocator_group("geos::algorithm::locate::SimplePointInAreaLocator");

template<>
template<>
void object::test<1>()
{
    auto g = reader.read("MULTISURFACE( CURVEPOLYGON( CIRCULARSTRING( 0 0, 4 0, 4 4, 0 4, 0 0), (1 1, 3 3, 3 1, 1 1)), ((10 10, 14 12, 11 10, 10 10), (11 11, 11.5 11, 11 11.5, 11 11)))");

    checkLocation(*g, {-2, 2}, Location::EXTERIOR); // outside envelope
    checkLocation(*g, {0, -0.25}, Location::EXTERIOR); // inside envelope, outside shell
    checkLocation(*g, {1, 1}, Location::BOUNDARY); // vertex of hole
    checkLocation(*g, {2, 1}, Location::BOUNDARY); // boundary of hole
    checkLocation(*g, {2.5, 1.5}, Location::EXTERIOR); // inside hole

}

}
