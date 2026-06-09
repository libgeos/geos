//
// Test Suite for geos::operation::distance::DistanceOp class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/distance/DistanceOp.h>
#include <geos/constants.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/util.h>
// std
#include <memory>
#include <string>

#include "utility.h"

using XY = geos::geom::CoordinateXY;
using geos::operation::distance::DistanceOp;
using geos::operation::distance::GeometryLocation;

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_distanceop_data {
    geos::io::WKTReader wktreader;

    typedef geos::geom::Geometry::Ptr GeomPtr;
    typedef std::unique_ptr<geos::geom::CoordinateSequence> CSPtr;

    test_distanceop_data()
        : wktreader()
    {}

    static void checkDistance(const Geometry& g1, const Geometry& g2, double expected, double tol = 0)
    {
        ensure_equals("distance is incorrect", g1.distance(&g2), expected, tol);
        ensure("unexpected isWithinDistance result for distance + tol", DistanceOp::isWithinDistance(g1, g2, expected + tol));
        if (expected > tol) {
            ensure("unexpected isWithinDistance result for distance - tol", !DistanceOp::isWithinDistance(g1, g2, expected - tol));
        }

        ensure_equals("reverse distance is incorrect", g2.distance(&g1), expected, tol);
    }

    static void checkLocation(const GeometryLocation& loc, const Geometry* component, std::size_t index, const geos::geom::CoordinateXY& c, double tol = 0)
    {
        ensure_equals("incorrect component", loc.getGeometryComponent(), component);
        ensure_equals("incorrect position", loc.getSegmentIndex(), index);
        ensure_equals_xy(loc.getCoordinate(), c, tol);
    }

    static void printLocs(const std::array<geos::operation::distance::GeometryLocation, 2>& locs)
    {
        if (locs[0].getCoordinate().equals2D(locs[1].getCoordinate())) {
            std::cout << "POINT (" << locs[0].getCoordinate() << ")" << std::endl;

        } else {
            std::cout << "LINESTRING (" << locs[0].getCoordinate() << ", " << locs[1].getCoordinate() << ")" << std::endl;
        }
        std::cout << locs[0].getCoordinate().distance(locs[1].getCoordinate()) << std::endl;
    }
};

typedef test_group<test_distanceop_data> group;
typedef group::object object;

group test_distanceop_group("geos::operation::distance::DistanceOp");

//
// Test Cases
//
template<>
template<>
void object::test<1>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(0 0)");
    std::string wkt1("POINT(10 0)");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 10);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(0, 0));
    ensure_equals(cs->getAt(1), Coordinate(10, 0));
}

template<>
template<>
void object::test<2>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(0 0)");
    std::string wkt1("MULTIPOINT((10 0), (50 30))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 10);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(0, 0));
    ensure_equals(cs->getAt(1), Coordinate(10, 0));

}

template<>
template<>
void object::test<3>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(3 0)");
    std::string wkt1("LINESTRING(0 10, 50 10, 100 50)");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 10);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(3, 0));
    ensure_equals(cs->getAt(1), Coordinate(3, 10));

}

template<>
template<>
void object::test<4>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(3 0)");
    std::string wkt1("MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 10);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(3, 0));
    ensure_equals(cs->getAt(1), Coordinate(3, 10));

}

template<>
template<>
void object::test<5>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(35 60)");
    std::string wkt1("POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 6);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(35, 60));
    ensure_equals(cs->getAt(1), Coordinate(35, 54));

}

template<>
template<>
void object::test<6>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(35 60)");
    std::string
    wkt1("MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 6);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(35, 60));
    ensure_equals(cs->getAt(1), Coordinate(35, 54));

}

template<>
template<>
void object::test<7>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(35 60)");

    // This is an invalid geom... anyway
    std::string
    wkt1("GEOMETRYCOLLECTION(MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)) ), POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)), MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50)), LINESTRING(0 10, 50 10, 100 50), MULTIPOINT((10 0), (50 30)), POINT(10 0))");

    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 6);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(35, 60));
    ensure_equals(cs->getAt(1), Coordinate(35, 54));
}

template<>
template<>
void object::test<8>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POINT(35 60)");

    // This is an invalid geom... anyway
    std::string wkt1("GEOMETRYCOLLECTION EMPTY");

    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure(std::isinf(dist.distance()));

    ensure(dist.nearestPoints() == nullptr);
}

template<>
template<>
void object::test<9>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((10 0), (50 30))");
    std::string wkt1("POINT(10 0)");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 0);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(10, 0));
    ensure_equals(cs->getAt(1), Coordinate(10, 0));

}

template<>
template<>
void object::test<10>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((10 0), (50 30))");
    std::string wkt1("MULTIPOINT((0 0), (150 30))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 10);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(10, 0));
    ensure_equals(cs->getAt(1), Coordinate(0, 0));

}

template<>
template<>
void object::test<11>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((3 0), (200 30))");
    std::string wkt1("LINESTRING(0 10, 50 10, 100 50)");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 10);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(3, 0));
    ensure_equals(cs->getAt(1), Coordinate(3, 10));

}

template<>
template<>
void object::test<12>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((3 0), (-50 30))");
    std::string wkt1("MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 10);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(3, 0));
    ensure_equals(cs->getAt(1), Coordinate(3, 10));

}

template<>
template<>
void object::test<13>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((-100 0), (35 60))");
    std::string wkt1("POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 6);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(35, 60));
    ensure_equals(cs->getAt(1), Coordinate(35, 54));

}

template<>
template<>
void object::test<14>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((-100 0), (35 60))");
    std::string
    wkt1("MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)))");
    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 6);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(35, 60));
    ensure_equals(cs->getAt(1), Coordinate(35, 54));
}

template<>
template<>
void object::test<15>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((-100 0), (35 60))");

    // This is an invalid geom... anyway
    std::string
    wkt1("GEOMETRYCOLLECTION(MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)) ), POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)), MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50)), LINESTRING(0 10, 50 10, 100 50), MULTIPOINT((10 0), (50 30)), POINT(10 0))");

    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure_equals(dist.distance(), 6);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(35, 60));
    ensure_equals(cs->getAt(1), Coordinate(35, 54));

}

template<>
template<>
void object::test<16>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("MULTIPOINT((-100 0), (35 60))");

    // This is an invalid geom... anyway
    std::string wkt1("GEOMETRYCOLLECTION EMPTY");

    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());

    ensure(std::isinf(dist.distance()));

    ensure(dist.nearestPoints() == nullptr);
}

// Test for crash reported in Ticket #236:
// http://trac.osgeo.org/geos/ticket/236
template<>
template<>
void object::test<17>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))");
    std::string wkt1("POLYGON((1.25 0.25, 1.25 0.75, 1.75 0.75, 1.75 0.25, 1.25 0.25))");

    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    DistanceOp dist(g0.get(), g1.get());
    ensure_equals(dist.distance(), 0.25);

    CSPtr cs(dist.nearestPoints());
    ensure_equals(cs->getAt(0), Coordinate(1, 0.25));
    ensure_equals(cs->getAt(1), Coordinate(1.25, 0.25));
}

// Test for isWithinDistance
template<>
template<>
void object::test<18>
()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;

    std::string wkt0("POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))");
    std::string wkt1("POLYGON((1.25 0.25, 1.25 0.75, 1.75 0.75, 1.75 0.25, 1.25 0.25))");

    GeomPtr g0(wktreader.read(wkt0));
    GeomPtr g1(wktreader.read(wkt1));

    ensure_equals(DistanceOp::distance(*g0, *g1), 0.25);
    ensure(DistanceOp::isWithinDistance(*g0, *g1, 0.26));
    ensure(!DistanceOp::isWithinDistance(*g0, *g1, 0.24));

    wkt0 = "LINESTRING(0 0, 0 1, 1 1, 1 0, 0 0)";
    wkt1 = "LINESTRING(2 0, 10 1, 10 10)";
    g0 = wktreader.read(wkt0);
    g1 = wktreader.read(wkt1);

    ensure_equals(DistanceOp::distance(*g0, *g1), 1);
    ensure(DistanceOp::isWithinDistance(*g0, *g1, 2));
    ensure(!DistanceOp::isWithinDistance(*g0, *g1, 0.8));

    // TODO: test closestPoints
}

// Test case submitted to Ticket #367
// http://trac.osgeo.org/geos/ticket/367/
template<>
template<>
void object::test<19>
()
{
    using geos::geom::GeometryFactory;
    const char* wkb_geom1 = "01060000000100000001030000000100000000000000";
    const char* wkb_geom2 = "010100000000000000000000000000000000000000";

    geos::geom::PrecisionModel precision(geos::geom::PrecisionModel::FLOATING);
    GeometryFactory::Ptr f(GeometryFactory::create(&precision));
    std::istringstream istr1(wkb_geom1);
    std::istringstream istr2(wkb_geom2);
    geos::io::WKBReader wkb(*f);
    GeomPtr g1(wkb.readHEX(istr1));
    GeomPtr g2(wkb.readHEX(istr2));
    ensure(g1->isValid());
    ensure(g2->isValid());

    ensure(std::isinf(g1->distance(g2.get())));
}

// Test case reported in Shapely
// https://github.com/Toblerity/Shapely/issues/560
template<>
template<>
void object::test<20>()
{
    using geos::operation::distance::DistanceOp;
    using geos::geom::Coordinate;
    using geos::geom::LineSegment;
    using geos::geom::Geometry;
    using geos::geom::LineString;
    using geos::geom::GeometryFactory;
    using geos::geom::CoordinateSequence;

    auto gfact = GeometryFactory::create();

    auto seq0 = geos::detail::make_unique<CoordinateSequence>(2u);
    auto seq1 = geos::detail::make_unique<CoordinateSequence>(2u);

    Coordinate a0{1, 5.0/3.0};
    Coordinate a1{2, 10.0/3.0};

    Coordinate b0{3, 5};
    Coordinate b1{0, 0};

    seq0->setAt(a0, 0);
    seq0->setAt(a1, 1);

    seq1->setAt(b0, 0);
    seq1->setAt(b1, 1);

    auto g0 = gfact->createLineString(std::move(seq0));
    auto g1 = gfact->createLineString(std::move(seq1));

    DistanceOp dist(g0.get(), g1.get());
    CSPtr seq(dist.nearestPoints());

    // input lines overlap, so generated point should intersect both geometries
    ensure(geos::geom::LineSegment(a0, a1).distance(seq->getAt(0)) < 1e-8);
    ensure(geos::geom::LineSegment(a0, a1).distance(seq->getAt(1)) < 1e-8);
    ensure(geos::geom::LineSegment(b0, b1).distance(seq->getAt(0)) < 1e-8);
    ensure(geos::geom::LineSegment(b0, b1).distance(seq->getAt(1)) < 1e-8);

    // reverse argument order and check again
    DistanceOp dist2(g1.get(), g0.get());
    seq = dist2.nearestPoints();

    ensure(geos::geom::LineSegment(a0, a1).distance(seq->getAt(0)) < 1e-8);
    ensure(geos::geom::LineSegment(a0, a1).distance(seq->getAt(1)) < 1e-8);
    ensure(geos::geom::LineSegment(b0, b1).distance(seq->getAt(0)) < 1e-8);
    ensure(geos::geom::LineSegment(b0, b1).distance(seq->getAt(1)) < 1e-8);
}

// JTS testDisjointCollinearSegments
template<>
template<>
void object::test<21>()
{
    auto g1 = wktreader.read("LINESTRING (0.0 0.0, 9.9 1.4)");
    auto g2 = wktreader.read("LINESTRING (11.88 1.68, 21.78 3.08)");

    ensure_equals(g1->distance(g2.get()), 1.9996999774966246);
}

//
// Variations on a theme: testing EMPTY and collections with EMPTY
//

// Ignoring empty component
template<>
template<>
void object::test<22>()
{
    auto g1 = wktreader.read("GEOMETRYCOLLECTION (POINT EMPTY, LINESTRING (0 0, 1 1))");
    auto g2 = wktreader.read("POINT (1 2)");

    ensure_equals(g1->distance(g2.get()), 1);
    ensure_equals(g2->distance(g1.get()), 1);
}

template<>
template<>
void object::test<23>()
{
    auto g1 = wktreader.read("POINT EMPTY");
    auto g2 = wktreader.read("LINESTRING EMPTY");

    ensure(g1 != nullptr && g2 != nullptr);
    ensure(std::isinf(g1->distance(g2.get())));
    ensure(std::isinf(g2->distance(g1.get())));
}

template<>
template<>
void object::test<24>()
{
    auto g1 = wktreader.read("GEOMETRYCOLLECTION(POINT EMPTY, LINESTRING EMPTY)");
    auto g2 = wktreader.read("LINESTRING EMPTY");

    ensure(g1 != nullptr && g2 != nullptr);
    ensure(std::isinf(g1->distance(g2.get())));
    ensure(std::isinf(g2->distance(g1.get())));
}

// But ignore empty if there's a real distance?
template<>
template<>
void object::test<25>()
{
    auto g1 = wktreader.read("GEOMETRYCOLLECTION(LINESTRING EMPTY, POINT(2 1))");
    auto g2 = wktreader.read("POINT(1 1)");

    ensure(g1 != nullptr && g2 != nullptr);
    ensure_equals(g1->distance(g2.get()), 1);
    ensure_equals(g2->distance(g1.get()), 1);
}

template<>
template<>
void object::test<26>()
{
    auto g1 = wktreader.read("GEOMETRYCOLLECTION(POINT(-2 0), POINT EMPTY)");
    auto g2 = wktreader.read("GEOMETRYCOLLECTION(POINT(1 0),LINESTRING(0 0,1 0))");

    ensure(g1 != nullptr && g2 != nullptr);
    ensure_equals(g1->distance(g2.get()), 2);
    ensure_equals(g2->distance(g1.get()), 2);
}

template<>
template<>
void object::test<27>()
{
    auto g1 = wktreader.read("GEOMETRYCOLLECTION(POINT EMPTY)");
    auto g2 = wktreader.read("GEOMETRYCOLLECTION(POINT(1 0))");

    ensure(g1 != nullptr && g2 != nullptr);
    ensure(std::isinf(g1->distance(g2.get())));
    ensure(std::isinf(g2->distance(g1.get())));
}

template<>
template<>
void object::test<28>()
{
    set_test_name("CircularString / LineString (disjoint)");

    auto g1 = wktreader.read("CIRCULARSTRING (-7 0, -6 -1, -5 0, 0 5, 4 -3)");
    auto g2 = wktreader.read("LINESTRING (4 -5, 7 0, 13 0)");

    checkDistance(*g1, *g2, 1.0024502, 1e-6);

    auto locs = DistanceOp(*g1, *g2).nearestLocations();

    checkLocation(locs[0], g1.get(), 2, XY(4.28746, -2.57248), 1e-5);
    checkLocation(locs[1], g2.get(), 0, XY(5.14706, -3.08824), 1e-5);
}

template<>
template<>
void object::test<29>()
{
    set_test_name("CircularString / LineString (intersecting)");

    auto g1 = wktreader.read("CIRCULARSTRING (-6 1, -6 -1, -5 0, 0 5, 4 -3)");
    auto g2 = wktreader.read("LINESTRING (-4 2, 4 -2, 6 0)");

    checkDistance(*g1, *g2, 0);

    auto locs = DistanceOp(*g1, *g2).nearestLocations();

    checkLocation(locs[0], g1.get(), 2, XY(4.87083, -1.12917), 1e-5);
    checkLocation(locs[1], g2.get(), 1, XY(4.87083, -1.12917), 1e-5);
}

template<>
template<>
void object::test<30>()
{
    set_test_name("CircularString / MultiPoint (disjoint)");

    auto g1 = wktreader.read("CIRCULARSTRING (-7 0, -6 -1, -5 0, 0 5, 4 -3)");
    auto g2 = wktreader.read("MULTIPOINT (3.5 -5, 0 2, 6 4)");

    checkDistance(*g1, *g2, 2.061553, 1e-6);

    auto locs = DistanceOp(*g1, *g2).nearestLocations();

    checkLocation(locs[0], g1.get(), 2, XY(4, -3), 1e-5);
    checkLocation(locs[1], g2->getGeometryN(0), 0, XY(3.5, -5), 1e-5);
}

template<>
template<>
void object::test<31>()
{
    set_test_name("CircularString / MultiPoint (intersecting)");

    auto g1 = wktreader.read("CIRCULARSTRING (-7 0, -6 -1, -5 0, 0 5, 4 -3)");
    auto g2 = wktreader.read("MULTIPOINT (-6 15, -6 -1)");

    checkDistance(*g1, *g2, 0);

    auto locs = DistanceOp(*g1, *g2).nearestLocations();

    checkLocation(locs[0], g1.get(), 0u, XY{-6, -1}, 1e-5);
    checkLocation(locs[1], g2->getGeometryN(1), 0u, XY{-6, -1}, 1e-5);
}

template<>
template<>
void object::test<32>()
{
    set_test_name("CircularString / CircularString (disjoint)");

    auto g1 = wktreader.read("CIRCULARSTRING (-5 0, 4 3, 4 -3, 3 0, 0 2, -2 0, -5 0)");
    auto g2 = wktreader.read("CIRCULARSTRING (0 -2, -1 -1.5, -2 -2, 1 -1, 2 -2)");

    checkDistance(*g1, *g2, 1.33846, 1e-5);

    auto locs = DistanceOp(*g1, *g2).nearestLocations();

    checkLocation(locs[0], g1.get(), 4, XY{-1.69468, 0.147266}, 1e-5);
    checkLocation(locs[1], g2.get(), 2, XY{-1.06012, -1.03121}, 1e-5);
}

template<>
template<>
void object::test<33>()
{
    set_test_name("CircularString / CircularString (intersecting)");

    auto g1 = wktreader.read("CIRCULARSTRING (-5 0, 4 3, 4 -3, 3 0, 0 2, -2 0, -5 0)");
    auto g2 = wktreader.read("CIRCULARSTRING (0 -2, -1 -2.5, -2 -2, 1 -1, 4 -2)");

    checkDistance(*g1, *g2, 0);

    auto locs = DistanceOp(*g1, *g2).nearestLocations();

    checkLocation(locs[0], g1.get(), 2, XY{3.86825, -1.9045}, 1e-5);
    checkLocation(locs[1], g2.get(), 2, XY{3.86825, -1.9045}, 1e-5);
}

template<>
template<>
void object::test<34>()
{
    set_test_name("CircularString / CompoundCurve (disjoint)");

    auto g1 = wktreader.read("CIRCULARSTRING (-5 0, 4 3, 4 -3, 3 0, 0 2, -2 0, -5 0)");
    auto g2 = wktreader.read<geos::geom::CompoundCurve>("COMPOUNDCURVE(CIRCULARSTRING (0 -2, -1 -2.5, -2 -2), (-2 -2, -10 0, -5 5))");

    checkDistance(*g1, *g2, 1.16409, 1e-5);

    auto locs = DistanceOp(*g1, *g2).nearestLocations();

    checkLocation(locs[0], g1.get(), 4, XY{-4.42355, -0.194193}, 1e-5);
    checkLocation(locs[1], g2->getCurveN(1), 0, XY{-4.70588, -1.32353}, 1e-5);
}

template<>
template<>
void object::test<35>()
{
    set_test_name("MultiPoint partially in CurvePolygon");

    auto g1 = wktreader.read("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING(-5 0, 0 5, 5 0), (5 0, -5 0)))");
    auto g2 = wktreader.read("MULTIPOINT (5 5, 1 1)");

    checkDistance(*g1, *g2, 0);
}

template<>
template<>
void object::test<36>()
{
    set_test_name("CurvePolygon contained in CurvePolygon");

    auto g1 = wktreader.read("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING(-5 0, 0 5, 5 0), (5 0, 0 -5, -5 0)))");
    auto g2 = wktreader.read("CURVEPOLYGON (CIRCULARSTRING (-1 0, 0 1, 1 0, 0 -1, -1 0))");

    checkDistance(*g1, *g2, 0);
}

template<>
template<>
void object::test<40>()
{
    set_test_name("CurvePolygon and point, PostGIS issue #5989");

    auto g1 = wktreader.read("CURVEPOLYGON(COMPOUNDCURVE((129296 142584,94722 100435,91618 97138,57306 60686,26874 28357,13059 34228,14572 65506,14593 65948,14616 66389),CIRCULARSTRING(14616 66389,17955 101124,24417 135415,24655 136418,24895 137421),(24895 137421,25472 139809,19354 141285,0 0,148000 142000,129296 142584)))");
    auto g2 = wktreader.read("POINT(19925 112376)");

    checkDistance(*g1, *g2, 199.655, 0.001);
}

template<>
template<>
void object::test<41>()
{
    set_test_name("CircularString and Point; PostGIS");

    auto g1 = wktreader.read("CIRCULARSTRING(-1 0, 0 1, 1 0)");
    auto g2 = wktreader.read("POINT (0 0)");

    checkDistance(*g1, *g2, 1, 1e-6);
}

template<>
template<>
void object::test<42>()
{
    set_test_name("CircularString and Point (2); PostGIS");

    auto g1 = wktreader.read("CIRCULARSTRING(-3 0, -2 0, -1 0, 0 1, 1 0)");
    auto g2 = wktreader.read("POINT (0 0)");

    checkDistance(*g1, *g2, 1, 1e-6);
}

template<>
template<>
void object::test<43>()
{
    set_test_name("CircularString and CircularString; PostGIS");

    auto g1 = wktreader.read("CIRCULARSTRING(-1 0, 0 1, 1 0)");
    auto g2 = wktreader.read( "CIRCULARSTRING(0 0, 1 -1, 2 0)");

    checkDistance(*g1, *g2, 1, 1e-6);
}

template<>
template<>
void object::test<44>()
{
    set_test_name("CurvePolygon and Point; PostGIS");

    auto g1 = wktreader.read( "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(1 6, 6 1, 9 7),(9 7, 3 13, 1 6)),COMPOUNDCURVE((3 6, 5 4, 7 4, 7 6),CIRCULARSTRING(7 6,5 8,3 6)))");

    auto p1 = wktreader.read("POINT(3 14)");
    auto p2 = wktreader.read("POINT(3 8)");
    auto p3 = wktreader.read("POINT(6 5)");
    auto p4 = wktreader.read("POINT(6 4)");

    checkDistance(*g1, *p1, 1, 1e-6);
    checkDistance(*g1, *p2, 0, 1e-6);
    checkDistance(*g1, *p3, 1, 1e-6);
    checkDistance(*g1, *p4, 0, 1e-6);
}

template<>
template<>
void object::test<45>()
{
    set_test_name("CurvePolygon and LineString; PostGIS");

    auto g1 = wktreader.read( "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(1 6, 6 1, 9 7),(9 7, 3 13, 1 6)),COMPOUNDCURVE((3 6, 5 4, 7 4, 7 6),CIRCULARSTRING(7 6,5 8,3 6)))");

    auto ls1 = wktreader.read("LINESTRING(0 0, 50 0)");
    auto ls2 = wktreader.read( "LINESTRING(6 0, 10 7)");
    auto ls3 = wktreader.read( "LINESTRING(4 4, 4 8)");
    auto ls4 = wktreader.read( "LINESTRING(4 7, 5 6, 6 7)");
    auto ls5 = wktreader.read( "LINESTRING(10 0, 10 2, 10 0)");

    checkDistance(*g1, *ls1, 0.917484, 1e-6);
    checkDistance(*g1, *ls2, 0, 1e-6);
    checkDistance(*g1, *ls3, 0, 1e-6);
    checkDistance(*g1, *ls4, 0.585786, 1e-6);
    checkDistance(*g1, *ls5, 1.52913, 1e-6);
}
    
template<>
template<>
void object::test<46>()
{
    set_test_name("CurvePolygon and Polygon; PostGIS");

    auto g1 = wktreader.read( "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(1 6, 6 1, 9 7),(9 7, 3 13, 1 6)),COMPOUNDCURVE((3 6, 5 4, 7 4, 7 6),CIRCULARSTRING(7 6,5 8,3 6)))");
    
    auto p1 = wktreader.read( "POLYGON((10 4, 10 8, 13 8, 13 4, 10 4))");
    auto p2 = wktreader.read( "POLYGON((9 4, 9 8, 12 8, 12 4, 9 4))");
    auto p3 = wktreader.read( "POLYGON((1 4, 1 8, 4 8, 4 4, 1 4))");
    
    checkDistance(*g1, *p1, 0.58415, 1e-6);
    checkDistance(*g1, *p2, 0);
    checkDistance(*g1, *p3, 0);
}

template<>
template<>
void object::test<47>()
{
    set_test_name("CurvePolygon and CurvePolygon; PostGIS");

    auto g1 = wktreader.read( "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(1 6, 6 1, 9 7),(9 7, 3 13, 1 6)),COMPOUNDCURVE((3 6, 5 4, 7 4, 7 6),CIRCULARSTRING(7 6,5 8,3 6)))");

    auto cp1 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(-1 4, 0 5, 1 4, 0 3, -1 4))");
    auto cp2 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(1 4, 2 5, 3 4, 2 3, 1 4))");

    checkDistance(*g1, *cp1, 0.0475666, 1e-6);
    checkDistance(*g1, *cp2, 0);
}

template<>
template<>
void object::test<48>()
{
    set_test_name("MultiSurface and CurvePolygon; PostGIS");

    auto g1 = wktreader.read( "MULTISURFACE(POLYGON((0 0,0 4,4 4,4 0,0 0)),CURVEPOLYGON(CIRCULARSTRING(8 2,10 4,12 2,10 0,8 2)))");

    auto cp1 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(5 2,6 3,7 2,6 1,5 2))");
    auto cp2 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(4 2,5 3,6 2,5 1,4 2))");
    auto cp3 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(5 3,6 2,5 1,4 2,5 3))");
    auto cp4 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(4.5 3,5.5 2,4.5 1,3.5 2,4.5 3))");
    auto cp5 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(5.5 3,6.5 2,5.5 1,4.5 2,5.5 3))");
    auto cp6 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(10 3,11 2,10 1,9 2,10 3))");
    auto cp7 = wktreader.read( "CURVEPOLYGON(CIRCULARSTRING(2 3,3 2,2 1,1 2,2 3))");
    auto cp8 = wktreader.read("CURVEPOLYGON(CIRCULARSTRING(5 7,6 8,7 7,6 6,5 7))");

    checkDistance(*g1, *cp1, 1, 1e-6);
    checkDistance(*g1, *cp2, 0);
    checkDistance(*g1, *cp3, 0);
    checkDistance(*g1, *cp4, 0);
    checkDistance(*g1, *cp5, 0.5, 1e-6);
    checkDistance(*g1, *cp6, 0);
    checkDistance(*g1, *cp7, 0);
    checkDistance(*g1, *cp8, 2.605551, 1e-6);
}

template<>
template<>
void object::test<49>()
{
    set_test_name("MultiCurve and LineString; PostGIS");

    auto g1 = wktreader.read("LINESTRING(0.5 1,0.5 3)");
    auto g2 = wktreader.read( "MULTICURVE(CIRCULARSTRING(2 3,3 2,2 1,1 2,2 3),(0 0, 0 5))");

    checkDistance(*g1, *g2, 0.5, 1e-6);
}

template<>
template<>
void object::test<50>()
{
    set_test_name("CurvePolygon and Point; PostGIS ticket #4326");

    auto g1 = wktreader.read("CURVEPOLYGON(CIRCULARSTRING(7874821 8715927,8907663 8715927,8844683 7750316,7937800 7750316,7874821 8715927))");
    auto g2 = wktreader.read("POINT(5433865 8243495)");

    checkDistance(*g1, *g2, 2271704.2698450615, 1e-6);
}

template<>
template<>
void object::test<51>()
{
    set_test_name("CurvePolygon and Point; PostGIS ticket #5989");

    auto g1 = wktreader.read("CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0, -1 5, 0 10), (0 10, -10 10, -10 0, 0 0)))");
    auto g2 = wktreader.read("POINT(-0.5 5)");

    checkDistance(*g1, *g2, 0.5, 1e-6);
}

template<>
template<>
void object::test<52>()
{
    set_test_name("MultiSurface COMPOUNDCURVE and Polygon; PostGIS ticket #5989 reopened (Finnish coordinates)");

    auto g1 = wktreader.read("MULTISURFACE(CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(25493681.3085 6678739.6419,25493637.8256 6678776.2541,25493599.9716 6678818.6604),(25493599.9716 6678818.6604,25493583.8816 6678839.494,25493590.9591 6678844.9594,25493566.9698 6678851.9051,25493526.5793 6678861.0985,25493487.3853 6678868.2546,25493447.392 6678871.7846,25493429.5033 6678876.6846,25493435.0147 6678857.4594,25493454.9254 6678821.6192,25493452.4867 6678816.1292,25493402.3226 6678828.2153,25493354.7526 6678837.8291,25493349.0217 6678838.9873,25493310.914 6678843.6926,25493306.3784 6678859.9295,25493294.1011 6678858.7864,25493169.9028 6678847.2212,25493127.549 6678843.2773,25493057.8888 6678834.6763,25493017.4547 6678826.7261,25492928.2395 6678821.5524,25492909.1067 6678795.0681,25492946.9544 6678794.8193,25492967.1292 6678794.6866,25493010.8878 6678795.9163,25493111.5564 6678804.3109,25493113.4069 6678799.5694,25493134.2375 6678806.0729),CIRCULARSTRING(25493134.2375 6678806.0729,25493223.038 6678812.0533,25493311.4919 6678802.1948,25493317.3589 6678800.9679,25493323.2107 6678799.6707),(25493323.2107 6678799.6707,25493362.6615 6678792.3924,25493364.6948 6678785.173,25493375.0512 6678781.6008,25493404.9736 6678771.2798),CIRCULARSTRING(25493404.9736 6678771.2798,25493486.8049 6678737.6728,25493566.8862 6678700.0859),(25493566.8862 6678700.0859,25493574.903 6678714.0761,25493600.2214 6678702.8803,25493654.1114 6678670.2493,25493669.7219 6678659.2147,25493677.0288 6678653.353,25493684.6468 6678648.4119,25493724.0447 6678623.7638,25493800.1925 6678583.5847,25493805.6261 6678586.1292,25493811.9258 6678582.3016,25493812.8413 6678583.0522,25493846.581 6678558.6541,25493884.2653 6678531.6272,25493900.9168 6678580.1258,25493830.7852 6678630.9518,25493789.0848 6678661.2736,25493746.1612 6678692.485,25493686.5413 6678735.8369,25493681.3085 6678739.6419))))");
    auto g2 = wktreader.read("POLYGON ((25492929.752797972 6678919.124091367,25493008.675235115 6678876.783133076,25493098.02917443 6678854.33589699,25493139.669701554 6678977.913508233,25493122.946292613 6679011.427315322,25493050.53388224 6679068.791126598,25493140.33593199 6679140.958135231,25492931.688606273 6679401.2946243705,25492799.289171 6679295.949040241,25492651.5177725 6679115.144800108,25492929.752797972 6678919.124091367))");

    checkDistance(*g1, *g2, 14.5, 1.0);
}


// TODO: finish the tests by adding:
// 	LINESTRING - *all*
// 	MULTILINESTRING - *all*
// 	POLYGON - *all*
// 	MULTIPOLYGON - *all*
// 	COLLECTION - *all*

} // namespace tut

