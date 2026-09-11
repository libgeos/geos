#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/geom/Location.h>
#include <geos/io/WKTReader.h>

using geos::geom::CoordinateXY;
using geos::geom::Location;
using geos::algorithm::locate::SimplePointInAreaLocator;

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

template<>
template<>
void object::test<2>()
{
    auto point = reader.read("POINT (1 1)");
    auto point2 = reader.read("POINT (100 100)");
    auto multiPoint = reader.read("MULTIPOINT (1 1, 11 11)");
    auto multiPoint2 = reader.read("MULTIPOINT (100 100, 200 200)");
    auto poly = reader.read("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    auto multiPoly = reader.read("MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0)), ((10 10, 20 10, 20 20, 10 20, 10 10)))");

    // isAnyPointContained
    ensure(SimplePointInAreaLocator::isAnyPointContained(*point, *poly));
    ensure(SimplePointInAreaLocator::isAnyPointContained(*point, *multiPoly));
    ensure(SimplePointInAreaLocator::isAnyPointContained(*multiPoint, *poly));
    ensure(SimplePointInAreaLocator::isAnyPointContained(*multiPoint, *multiPoly));
    ensure(!SimplePointInAreaLocator::isAnyPointContained(*point2, *poly));
    ensure(!SimplePointInAreaLocator::isAnyPointContained(*point2, *multiPoly));
    ensure(!SimplePointInAreaLocator::isAnyPointContained(*multiPoint2, *poly));
    ensure(!SimplePointInAreaLocator::isAnyPointContained(*multiPoint2, *multiPoly));

    // isEveryPointContained
    ensure(SimplePointInAreaLocator::isEveryPointContained(*point, *poly));
    ensure(SimplePointInAreaLocator::isEveryPointContained(*point, *multiPoly));
    ensure(!SimplePointInAreaLocator::isEveryPointContained(*multiPoint, *poly));
    ensure(SimplePointInAreaLocator::isEveryPointContained(*multiPoint, *multiPoly));
    ensure(!SimplePointInAreaLocator::isEveryPointContained(*point2, *poly));
    ensure(!SimplePointInAreaLocator::isEveryPointContained(*point2, *multiPoly));
    ensure(!SimplePointInAreaLocator::isEveryPointContained(*multiPoint2, *poly));
    ensure(!SimplePointInAreaLocator::isEveryPointContained(*multiPoint2, *multiPoly));

    // argument order reversed
    ensure_THROW(SimplePointInAreaLocator::isAnyPointContained(*poly, *point), geos::util::GEOSException);
    ensure_THROW(SimplePointInAreaLocator::isEveryPointContained(*poly, *point), geos::util::GEOSException);
}

}
