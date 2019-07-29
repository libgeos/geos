//
// Test Suite for geos::algorithm::PointLocator
// Ported from JTS junit/algorithm/PointLocator.java

#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/RayCrossingCounterDD.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h> // required for use in unique_ptr
#include <geos/geom/Polygon.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
// std
#include <sstream>
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

// dummy data, not used
struct test_locatepointinring_data {};

typedef test_group<test_locatepointinring_data> group;
typedef group::object object;

group test_locatepointinring_group("geos::algorithm::LocatePointInRing");

// These are static to avoid namespace pollution
// The struct test_*_data above is probably there
// for the same reason...
//
static PrecisionModel pm;
static GeometryFactory::Ptr gf = GeometryFactory::create(&pm);
static geos::io::WKTReader reader(gf.get());

static void
runPtLocator(Location expected, const Coordinate& pt,
             const std::string& wkt)
{
    std::unique_ptr<Geometry> geom(reader.read(wkt));
    const Polygon* poly = dynamic_cast<Polygon*>(geom.get());
    const CoordinateSequence* cs = poly->getExteriorRing()->getCoordinatesRO();
    Location loc = PointLocation::locateInRing(pt, *cs);
    ensure_equals(loc, expected);
}

static void
runPtLocatorDD(Location expected, const Coordinate& pt,
               const std::string& wkt)
{
    std::unique_ptr<Geometry> geom(reader.read(wkt));
    const Polygon* poly = dynamic_cast<Polygon*>(geom.get());
    const CoordinateSequence* cs = poly->getExteriorRing()->getCoordinatesRO();
    Location loc = RayCrossingCounterDD::locatePointInRing(pt, *cs);
    ensure_equals(loc, expected);
}

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
    runPtLocatorDD(Location::EXTERIOR,
                   Coordinate(25.374625374625374, 128.35564435564436),
                   "POLYGON ((0.0 0.0, 0.0 172.0, 100.0 0.0, 0.0 0.0))");
}

// 6 - robustness
template<>
template<>
void object::test<6>
()
{
    runPtLocatorDD(Location::INTERIOR,
                   Coordinate(97.96039603960396, 782.0),
                   "POLYGON ((642.0 815.0, 69.0 764.0, 394.0 966.0, 642.0 815.0))");
}

// 7 - robustness
template<>
template<>
void object::test<7>
()
{
    runPtLocatorDD(Location::EXTERIOR,
                   Coordinate(3.166572116932842, 48.5390194687463),
                   "POLYGON ((2.152214146946829 50.470470727186765, 18.381941666723034 19.567250592139274, 2.390837642830135 49.228045261718165, 2.152214146946829 50.470470727186765))");
}



} // namespace tut

