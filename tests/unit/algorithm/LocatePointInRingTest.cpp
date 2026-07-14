//
// Test Suite for geos::algorithm::PointLocator
// Ported from JTS junit/algorithm/PointLocator.java

#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h> // required for use in unique_ptr
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
// std
#include <string>
#include <memory>

namespace geos {
namespace geom {
class Geometry;
}
}

using namespace geos::geom; // for Location
using namespace geos::algorithm; // for Location

namespace tut {
//
// Test Group
//

struct test_locatepointinring_data {
    geos::io::WKTReader reader;

    std::string locationText(Location loc) {
        switch(loc) {
            case Location::BOUNDARY: return "BOUNDARY";
            case Location::EXTERIOR: return "EXTERIOR";
            case Location::INTERIOR: return "INTERIOR";
            default: return "NONE";
        }
    }

    void
    runPtLocator(Location expected, const CoordinateXY& pt,
             const std::string& wkt, bool checkReverse=true)
    {
        std::unique_ptr<Geometry> geom(reader.read(wkt));
        const Surface* poly = dynamic_cast<Surface*>(geom.get());
        const Curve* cs = poly->getExteriorRing();
        Location loc = PointLocation::locateInRing(pt, *cs);

        if (loc != expected) {
            std::string message = "Expected (" + pt.toString() + ") to be " + locationText(expected) + " but got " + locationText(loc) + " for " + wkt;
            fail(message);
        }

        if (checkReverse) {
            runPtLocator(expected, pt, poly->reverse()->toString(), false);
        }
    }
};

typedef test_group<test_locatepointinring_data> group;
typedef group::object object;

group test_locatepointinring_group("geos::algorithm::LocatePointInRing");

const std::string
wkt_comb("POLYGON ((0 0, 0 10, 4 5, 6 10, 7 5, 9 10, 10 5, 13 5, 15 10, 16 3, 17 10, 18 3, 25 10, 30 10, 30 0, 15 0, 14 5, 13 0, 9 0, 8 5, 6 0, 0 0))");
const std::string
wkt_rpts("POLYGON ((0 0, 0 10, 2 5, 2 5, 2 5, 2 5, 2 5, 3 10, 6 10, 8 5, 8 5, 8 5, 8 5, 10 10, 10 5, 10 5, 10 5, 10 5, 10 0, 0 0))");

//
// Test Cases
//

// 1 - Test box
template<>
template<>
void object::test<1>
()
{
    runPtLocator(
        Location::INTERIOR,
        Coordinate(10, 10),
        "POLYGON ((0 0, 0 20, 20 20, 20 0, 0 0))");
}

// 2 - Test complex ring
template<>
template<>
void object::test<2>
()
{
    runPtLocator(
        Location::INTERIOR,
        Coordinate(0, 0),
        "POLYGON ((-40 80, -40 -80, 20 0, 20 -100, 40 40, 80 -80, 100 80, 140 -20, 120 140, 40 180,     60 40, 0 120, -20 -20, -40 80))");
}

// 3 - Comb tests
template<>
template<>
void object::test<3>
()
{
    runPtLocator(Location::BOUNDARY, Coordinate(0, 0), wkt_comb);
    runPtLocator(Location::BOUNDARY, Coordinate(0, 1), wkt_comb);
    // at vertex
    runPtLocator(Location::BOUNDARY, Coordinate(4, 5), wkt_comb);
    runPtLocator(Location::BOUNDARY, Coordinate(8, 5), wkt_comb);

    // on horizontal segment
    runPtLocator(Location::BOUNDARY, Coordinate(11, 5), wkt_comb);
    // on vertical segment
    runPtLocator(Location::BOUNDARY, Coordinate(30, 5), wkt_comb);
    // on angled segment
    runPtLocator(Location::BOUNDARY, Coordinate(22, 7), wkt_comb);

    runPtLocator(Location::INTERIOR, Coordinate(1, 5), wkt_comb);
    runPtLocator(Location::INTERIOR, Coordinate(5, 5), wkt_comb);
    runPtLocator(Location::INTERIOR, Coordinate(1, 7), wkt_comb);

    runPtLocator(Location::EXTERIOR, Coordinate(12, 10), wkt_comb);
    runPtLocator(Location::EXTERIOR, Coordinate(16, 5), wkt_comb);
    runPtLocator(Location::EXTERIOR, Coordinate(35, 5), wkt_comb);
}


// 4 - repeated points
template<>
template<>
void object::test<4>
()
{
    runPtLocator(Location::BOUNDARY, Coordinate(0, 0), wkt_rpts);
    runPtLocator(Location::BOUNDARY, Coordinate(0, 1), wkt_rpts);
    // at vertex
    runPtLocator(Location::BOUNDARY, Coordinate(2, 5), wkt_rpts);
    runPtLocator(Location::BOUNDARY, Coordinate(8, 5), wkt_rpts);
    runPtLocator(Location::BOUNDARY, Coordinate(10, 5), wkt_rpts);

    runPtLocator(Location::INTERIOR, Coordinate(1, 5), wkt_rpts);
    runPtLocator(Location::INTERIOR, Coordinate(3, 5), wkt_rpts);
}

// 5 - robustness
template<>
template<>
void object::test<5>
()
{
    runPtLocator(Location::EXTERIOR,
                 Coordinate(25.374625374625374, 128.35564435564436),
                 "POLYGON ((0.0 0.0, 0.0 172.0, 100.0 0.0, 0.0 0.0))");
}

// 6 - robustness
template<>
template<>
void object::test<6>
()
{
    runPtLocator(Location::INTERIOR,
                 Coordinate(97.96039603960396, 782.0),
                 "POLYGON ((642.0 815.0, 69.0 764.0, 394.0 966.0, 642.0 815.0))");
}

// 7 - robustness
template<>
template<>
void object::test<7>
()
{
    runPtLocator(Location::EXTERIOR,
                 Coordinate(3.166572116932842, 48.5390194687463),
                 "POLYGON ((2.152214146946829 50.470470727186765, 18.381941666723034 19.567250592139274, 2.390837642830135 49.228045261718165, 2.152214146946829 50.470470727186765))");
}

// basic curve
template<>
template<>
void object::test<8>
()
{
    std::vector<std::string> wkts{
        "CURVEPOLYGON (COMPOUNDCURVE((0 0, 0 2), CIRCULARSTRING (0 2, 1 1, 0 0)))",
        "CURVEPOLYGON (COMPOUNDCURVE((0 2, 0 0), CIRCULARSTRING (0 0, 1 1, 0 2)))",
        "CURVEPOLYGON (COMPOUNDCURVE(CIRCULARSTRING (0 2, 1 1, 0 0), (0 0, 0 2)))",
        "CURVEPOLYGON (COMPOUNDCURVE(CIRCULARSTRING (0 0, 1 1, 0 2), (0 2, 0 0)))",
    };

    for (const auto& wkt: wkts) {
        // left of shape
        runPtLocator(Location::EXTERIOR,
             CoordinateXY(-1, 0.5),
             wkt);

        // right of shape
        runPtLocator(Location::EXTERIOR,
             CoordinateXY(1.1, 0.5),
             wkt);

        // on line segment
        runPtLocator(Location::BOUNDARY,
             CoordinateXY(0, 0.5),
             wkt);

        // on vertex
        runPtLocator(Location::BOUNDARY,
             CoordinateXY(0, 0),
             wkt);

        // on vertex
        runPtLocator(Location::BOUNDARY,
             CoordinateXY(0, 2),
             wkt);

        // inside
        runPtLocator(Location::INTERIOR,
             CoordinateXY(0.5, 1),
             wkt);
    }
}

// more complex curve (curved version of #2)
template<>
template<>
void object::test<9>()
{
    std::string wkt = "CURVEPOLYGON (COMPOUNDCURVE ("
                                   "(-40 80, -40 -80),"
                                   "CIRCULARSTRING (-40 -80, 0 -50, 20 0),"
                                   "(20 0, 20 -100),"
                                   "CIRCULARSTRING (20 -100, 40 -30, 40 40, 70 -10, 80 -80, 95 0, 100 80, 115 35, 140 -20, 115 80, 120 140, 95 200, 40 180, 85 125, 60 40, 60 115, 0 120),"
                                   "(0 120, -10 120, -20 -20, -40 80)))";

    runPtLocator(Location::EXTERIOR, CoordinateXY(-50, 40), wkt);
    runPtLocator(Location::INTERIOR, CoordinateXY(39, 40), wkt);
    runPtLocator(Location::BOUNDARY, CoordinateXY(40, 40), wkt);
    runPtLocator(Location::BOUNDARY, CoordinateXY(60, 40), wkt);

    runPtLocator(Location::EXTERIOR, CoordinateXY(-20, 100), wkt);
    runPtLocator(Location::INTERIOR, CoordinateXY(0, 100), wkt);
    runPtLocator(Location::EXTERIOR, CoordinateXY(80, 100), wkt);
    runPtLocator(Location::INTERIOR, CoordinateXY(100, 100), wkt);
    runPtLocator(Location::EXTERIOR, CoordinateXY(130, 100), wkt);

    runPtLocator(Location::EXTERIOR, CoordinateXY(-15, 120), wkt);
    runPtLocator(Location::BOUNDARY, CoordinateXY(-10, 120), wkt);
    runPtLocator(Location::BOUNDARY, CoordinateXY(-5, 120), wkt);
    runPtLocator(Location::BOUNDARY, CoordinateXY(0, 120), wkt);
    runPtLocator(Location::INTERIOR, CoordinateXY(5, 120), wkt);
    runPtLocator(Location::EXTERIOR, CoordinateXY(75, 120), wkt);
    runPtLocator(Location::INTERIOR, CoordinateXY(100, 120), wkt);
    runPtLocator(Location::EXTERIOR, CoordinateXY(120, 120), wkt);
}

// horizontal ray is tangent to curve
template<>
template<>
void object::test<10>()
{
    std::string wkt = "CURVEPOLYGON (COMPOUNDCURVE(CIRCULARSTRING(0 0, 1 1, 2 0), (2 0, 0 0)))";

    runPtLocator(Location::EXTERIOR, CoordinateXY(0, 1), wkt);
    runPtLocator(Location::BOUNDARY, CoordinateXY(1, 1), wkt);
    runPtLocator(Location::EXTERIOR, CoordinateXY(1.1, 1), wkt);
}

// degenerate arc (collinear points)
template<>
template<>
void object::test<11>()
{
    std::string wkt = "CURVEPOLYGON (CIRCULARSTRING(0 0, 4 6, 10 10, 9 6, 8 2, 1 1, 0 0))";

    runPtLocator(Location::EXTERIOR, CoordinateXY(0, 7), wkt);
    runPtLocator(Location::EXTERIOR, CoordinateXY(0, 6), wkt);
    runPtLocator(Location::EXTERIOR, CoordinateXY(0, 5), wkt);
}

template<>
template<>
void object::test<12>()
{
    set_test_name("robustness test from PostGIS ticket #6023");

    std::string wkt = "POLYGON ((11.230120879533454 62.84897119848748,11.230120879533905 62.8489711984873,11.23020501303477 62.84900750109812,11.230170431987244 62.84904481447776,11.230117909393426 62.8489943480894,11.230120879533454 62.84897119848748))";

    runPtLocator(Location::BOUNDARY, CoordinateXY(11.230120879533454, 62.84897119848748), wkt);
}

template<>
template<>
void object::test<13>()
{
    std::string wkt = "CURVEPOLYGON(COMPOUNDCURVE ("
        "(220 140, 60 140, 60 260, 220 240),"
        "CIRCULARSTRING (220 240, 230 230, 220 220, 190 230, 160 220, 155 210, 160 200, 174.18861169915812 207.4341649025257, 190 210),"
        "(190 210, 190 180, 160 180, 160 160, 220 160, 220 140)))";

    for (int y = 195; y <= 245; y++) {
        runPtLocator(Location::INTERIOR, {80, static_cast<double>(y)}, wkt);
    }
}

template<>
template<>
void object::test<14>()
{
    std::string wkt = "CURVEPOLYGON(COMPOUNDCURVE ((-10 100, 0 110, 10 100), CIRCULARSTRING (10 100, 35 75, 10 50, 35 25, 10 0), (10 0, 0 -10, -10 0), CIRCULARSTRING (-10 0, -35 25, -10 50, -35 75, -10 100)))";

    runPtLocator(Location::EXTERIOR, {-20, 0},  wkt);
    runPtLocator(Location::INTERIOR, {0, 0},  wkt);
    runPtLocator(Location::EXTERIOR, {-20, 0},  wkt);

    runPtLocator(Location::EXTERIOR, {-20, 50},  wkt);
    runPtLocator(Location::INTERIOR, {0, 50},  wkt);
    runPtLocator(Location::EXTERIOR, {-20, 50},  wkt);

    runPtLocator(Location::EXTERIOR, {-20, 100},  wkt);
    runPtLocator(Location::INTERIOR, {0, 100},  wkt);
    runPtLocator(Location::EXTERIOR, {-20, 100},  wkt);
}

template<>
template<>
void object::test<15>()
{
    std::string wkt = "CURVEPOLYGON(COMPOUNDCURVE (CIRCULARSTRING(0 0, 10 10, 20 0), (20 0, 30 10, 40 0), CIRCULARSTRING (40 0, 50 -10, 60 0), (60 0, 70 -10, 80 0), CIRCULARSTRING (80 0, 70 10, 80 20), (80 20, 100 20), CIRCULARSTRING (100 20, 120 0, 100 -20), (100 -20, 100 0), CIRCULARSTRING (100 0, 50 -50, 0 0)))";

    // scanline at y = 0
    runPtLocator(Location::EXTERIOR, {-10, 0}, wkt);
    runPtLocator(Location::INTERIOR, {10, 0}, wkt);
    runPtLocator(Location::INTERIOR, {30, 0}, wkt);
    runPtLocator(Location::EXTERIOR, {50, 0}, wkt);
    runPtLocator(Location::EXTERIOR, {70, 0}, wkt);
    runPtLocator(Location::INTERIOR, {90, 0}, wkt);
    runPtLocator(Location::INTERIOR, {110, 0}, wkt);
    runPtLocator(Location::EXTERIOR, {130, 0}, wkt);

    // scanline at y = 5
    runPtLocator(Location::EXTERIOR, {0, 5}, wkt);
    runPtLocator(Location::INTERIOR, {10, 5}, wkt);
    runPtLocator(Location::EXTERIOR, {20, 5}, wkt);
    runPtLocator(Location::INTERIOR, {30, 5}, wkt);
    runPtLocator(Location::EXTERIOR, {40, 5}, wkt);
    runPtLocator(Location::EXTERIOR, {50, 5}, wkt);
    runPtLocator(Location::EXTERIOR, {60, 5}, wkt);
    runPtLocator(Location::EXTERIOR, {70, 5}, wkt);
    runPtLocator(Location::INTERIOR, {75, 5}, wkt);
    runPtLocator(Location::INTERIOR, {80, 5}, wkt);
    runPtLocator(Location::INTERIOR, {100, 5}, wkt);
    runPtLocator(Location::INTERIOR, {110, 5}, wkt);
    runPtLocator(Location::EXTERIOR, {120, 5}, wkt);
    runPtLocator(Location::EXTERIOR, {125, 5}, wkt);
}

} // namespace tut

