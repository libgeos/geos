//
// Test Suite for geos::algorithm::distance::DirectedHausdorffDistance
// Ported from JTS DirectedHausdorffDistanceTest (locationtech/jts#1182)
// plus the discrete-under-estimate witness from DiscreteHausdorffDistance.

#include <tut/tut.hpp>

#include <geos/algorithm/distance/DirectedHausdorffDistance.h>
#include <geos/algorithm/distance/DiscreteHausdorffDistance.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>

#include <cmath>
#include <memory>
#include <string>

using geos::algorithm::distance::DirectedHausdorffDistance;
using geos::algorithm::distance::DiscreteHausdorffDistance;
using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::PrecisionModel;

namespace tut {

struct test_DirectedHausdorffDistance_data {
    typedef std::unique_ptr<Geometry> GeomPtr;

    test_DirectedHausdorffDistance_data()
        : pm()
        , gf(GeometryFactory::create(&pm))
        , reader(gf.get())
    {}

    static constexpr double TOLERANCE = 0.001;

    GeomPtr
    read(const std::string& wkt)
    {
        return reader.read(wkt);
    }

    void
    checkDistance(const std::string& wkt1, const std::string& wkt2,
                  double expectedDistance)
    {
        GeomPtr g1 = read(wkt1);
        GeomPtr g2 = read(wkt2);
        double dist = DirectedHausdorffDistance::distance(*g1, *g2);
        ensure(std::fabs(dist - expectedDistance) <= TOLERANCE);
    }

    void
    checkDistance(const std::string& wkt1, const std::string& wkt2,
                  double tolerance, double expectedDistance)
    {
        GeomPtr g1 = read(wkt1);
        GeomPtr g2 = read(wkt2);
        double dist = DirectedHausdorffDistance::distance(*g1, *g2, tolerance);
        ensure(std::fabs(dist - expectedDistance) <= TOLERANCE);
    }

    void
    checkDistanceLine(const std::string& wkt1, const std::string& wkt2,
                      const std::string& wktExpected)
    {
        GeomPtr g1 = read(wkt1);
        GeomPtr g2 = read(wkt2);
        auto pts = DirectedHausdorffDistance::distancePoints(*g1, *g2);
        ensure(pts.has_value());
        GeomPtr expected = read(wktExpected);
        auto expCoords = expected->getCoordinates();
        auto exp0 = expCoords->getAt<CoordinateXY>(0);
        auto exp1 = expCoords->getAt<CoordinateXY>(1);
        ensure(std::fabs((*pts)[0].x - exp0.x) <= TOLERANCE);
        ensure(std::fabs((*pts)[0].y - exp0.y) <= TOLERANCE);
        ensure(std::fabs((*pts)[1].x - exp1.x) <= TOLERANCE);
        ensure(std::fabs((*pts)[1].y - exp1.y) <= TOLERANCE);
    }

    void
    checkHausdorffLine(const std::string& wkt1, const std::string& wkt2,
                       const std::string& wktExpected)
    {
        GeomPtr g1 = read(wkt1);
        GeomPtr g2 = read(wkt2);
        auto pts = DirectedHausdorffDistance::hausdorffDistancePoints(*g1, *g2);
        ensure(pts.has_value());
        GeomPtr expected = read(wktExpected);
        auto expCoords = expected->getCoordinates();
        auto exp0 = expCoords->getAt<CoordinateXY>(0);
        auto exp1 = expCoords->getAt<CoordinateXY>(1);
        ensure(std::fabs((*pts)[0].x - exp0.x) <= TOLERANCE);
        ensure(std::fabs((*pts)[0].y - exp0.y) <= TOLERANCE);
        ensure(std::fabs((*pts)[1].x - exp1.x) <= TOLERANCE);
        ensure(std::fabs((*pts)[1].y - exp1.y) <= TOLERANCE);
    }

    void
    checkDistanceEmpty(const std::string& a, const std::string& b)
    {
        GeomPtr g1 = read(a);
        GeomPtr g2 = read(b);
        ensure(!DirectedHausdorffDistance::distancePoints(*g1, *g2).has_value());
        ensure(std::isnan(DirectedHausdorffDistance::distance(*g1, *g2)));
        ensure(std::isnan(DirectedHausdorffDistance::hausdorffDistance(*g1, *g2)));
    }

    void
    checkFullyWithin(const std::string& a, const std::string& b,
                     double distance, bool expected)
    {
        GeomPtr g1 = read(a);
        GeomPtr g2 = read(b);
        bool result = DirectedHausdorffDistance::isFullyWithinDistance(*g1, *g2, distance);
        ensure_equals(result, expected);
    }

    PrecisionModel pm;
    GeometryFactory::Ptr gf;
    geos::io::WKTReader reader;
};

typedef test_group<test_DirectedHausdorffDistance_data> group;
typedef group::object object;

group test_DirectedHausdorffDistance_group(
    "geos::algorithm::distance::DirectedHausdorffDistance");

// 1 — empty operands yield NaN / missing pair
template<>
template<>
void object::test<1>()
{
    checkDistanceEmpty("POINT EMPTY", "POINT (1 1)");
    checkDistanceEmpty("LINESTRING EMPTY", "LINESTRING (0 0, 2 1)");
    checkDistanceEmpty("POLYGON EMPTY", "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))");
}

// 2 — negative tolerance throws
template<>
template<>
void object::test<2>()
{
    GeomPtr g1 = read("POINT (5 5)");
    GeomPtr g2 = read("LINESTRING (5 1, 9 5)");
    bool threw = false;
    try {
        DirectedHausdorffDistance::distance(*g1, *g2, -1.0);
    }
    catch (const geos::util::IllegalArgumentException&) {
        threw = true;
    }
    ensure(threw);
}

// 3 — point / line pins
template<>
template<>
void object::test<3>()
{
    checkDistance("POINT (3 4)", "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))", 0.0);
    checkHausdorffLine("POINT (0 0)", "POINT (1 1)", "LINESTRING (0 0, 1 1)");
    checkHausdorffLine("LINESTRING (0 0, 2 0)", "LINESTRING (0 0, 2 1)",
                       "LINESTRING (2 0, 2 1)");
    checkHausdorffLine("LINESTRING (0 0, 2 0)", "LINESTRING (0 1, 1 2, 2 1)",
                       "LINESTRING (1 0, 1 2)");
    checkHausdorffLine("LINESTRING (0 0, 2 0)", "MULTIPOINT (0 2, 1 0, 2 1)",
                       "LINESTRING (0 0, 0 2)");
}

// 4 — topologically equal lines have directed distance 0
template<>
template<>
void object::test<4>()
{
    checkDistance(
        "MULTILINESTRING ((10 10, 10 90, 40 30), (40 30, 60 80, 90 30, 40 10))",
        "LINESTRING (10 10, 10 90, 40 30, 60 80, 90 30, 40 10)",
        0.0);
}

// 5 — directed lines
template<>
template<>
void object::test<5>()
{
    checkDistanceLine(
        "LINESTRING (1 6, 3 5, 1 4)",
        "LINESTRING (1 10, 9 5, 1 2)",
        "LINESTRING (1 6, 2.797752808988764 8.876404494382022)");
    checkDistanceLine(
        "LINESTRING (1 10, 9 5, 1 2)",
        "LINESTRING (1 6, 3 5, 1 4)",
        "LINESTRING (9 5, 3 5)");
}

// 6 — line crossing a polygon: farthest is outside
template<>
template<>
void object::test<6>()
{
    checkDistanceLine(
        "LINESTRING (2 5, 5 10, 6 4)",
        "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
        "LINESTRING (5 10, 5 9)");
}

// 7 — interior segments of a nested square
template<>
template<>
void object::test<7>()
{
    checkDistance(
        "POLYGON ((4 6, 5 6, 5 5, 4 5, 4 6))",
        "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
        2.0, 0.0);
    checkDistance(
        "POLYGON ((1 9, 3 9, 4 5, 5.05 9, 9 9, 9 1, 1 1, 1 9))",
        "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
        0.0);
}

// 8 — isFullyWithinDistance
template<>
template<>
void object::test<8>()
{
    checkFullyWithin("POINT EMPTY", "MULTIPOINT ((1 1), (9 9))", 1, false);
    checkFullyWithin("MULTIPOINT ((1 9), (9 1))", "MULTIPOINT ((1 1), (9 9))", 1, false);
    checkFullyWithin("MULTIPOINT ((1 9), (9 1))", "MULTIPOINT ((1 1), (9 9))", 8.1, true);

    const std::string a = "MULTILINESTRING ((1 1, 3 3), (7 7, 9 9))";
    const std::string b = "MULTILINESTRING ((1 9, 1 5), (6 4, 8 2))";
    checkFullyWithin(a, b, 1, false);
    checkFullyWithin(a, b, 4, false);
    checkFullyWithin(a, b, 6, true);
}

// 9 — discrete under-estimate witness (the grill pair)
// On the spike (100 0)–(10 100), min-distance to B's two arms is equal at
// t = 11/19: point (910/19, 1100/19), distance 910/19 ≈ 47.89473684210526.
// Auto-tolerance is envelope diameter / 1e4 ≈ 0.014.
template<>
template<>
void object::test<9>()
{
    const std::string a = "LINESTRING (0 0, 100 0, 10 100, 10 100)";
    const std::string b = "LINESTRING (0 100, 0 10, 80 10)";
    GeomPtr g1 = read(a);
    GeomPtr g2 = read(b);

    double discrete = DiscreteHausdorffDistance::distance(*g1, *g2);
    double locus = DirectedHausdorffDistance::hausdorffDistance(*g1, *g2);

    ensure(discrete <= 22.360679774997898 + 1e-9);
    constexpr double LOCUS_HD = 910.0 / 19.0;
    constexpr double LOCUS_TOL = 0.05;
    ensure(std::fabs(locus - LOCUS_HD) <= LOCUS_TOL);
}

// 10 — polygon query: non-zero HD attained on the boundary
// (JTS testPolygonLineCrossingBoundaryResult). Test 7's nested 0 would
// also pass if edges were skipped; this pair cannot.
template<>
template<>
void object::test<10>()
{
    checkDistanceLine(
        "POLYGON ((2 8, 8 2, 2 1, 2 8))",
        "LINESTRING (6 5, 4 7, 0 0, 8 4)",
        "LINESTRING (2 8, 3.9384615384615387 6.892307692307693)");
    checkDistance(
        "POLYGON ((2 8, 8 2, 2 1, 2 8))",
        "LINESTRING (6 5, 4 7, 0 0, 8 4)",
        2.233);
}

// 11 — identical long linestring is zero (JTS isSameOrCollinear)
//
// Same-geometry queries skip the vertex walk via TargetDistance::isSameOrCollinear.
// Distance is the assertion. A wall-clock bound flakes under Valgrind memcheck
// (CI Debug all-unit-tests), which is why JTS's timed canary is not ported.
template<>
template<>
void object::test<11>()
{
    const std::size_t n = 2000;
    auto cs = std::make_unique<geos::geom::CoordinateSequence>();
    for (std::size_t i = 0; i < n; ++i) {
        cs->add(geos::geom::Coordinate(static_cast<double>(i), 0.0));
    }
    GeomPtr line(gf->createLineString(std::move(cs)));

    double dist = DirectedHausdorffDistance::distance(*line, *line);
    ensure(std::fabs(dist) <= TOLERANCE);
}

} // namespace tut
