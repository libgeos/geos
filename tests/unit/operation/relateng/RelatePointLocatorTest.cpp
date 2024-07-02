//
// Test Suite for geos::operation::relateng::RelatePointLocator class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/operation/relateng/RelatePointLocator.h>
#include <geos/operation/relateng/DimensionLocation.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::io::WKTReader;
// using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_relatepointlocator_data {

    WKTReader r;
    // WKTWriter w;

    std::string gcPLA =
        "GEOMETRYCOLLECTION (POINT (1 1), POINT (2 1), LINESTRING (3 1, 3 9), LINESTRING (4 1, 5 4, 7 1, 4 1), LINESTRING (12 12, 14 14), POLYGON ((6 5, 6 9, 9 9, 9 5, 6 5)), POLYGON ((10 10, 10 16, 16 16, 16 10, 10 10)), POLYGON ((11 11, 11 17, 17 17, 17 11, 11 11)), POLYGON ((12 12, 12 16, 16 16, 16 12, 12 12)))";

    // void
    // checkLocation(const std::string& wkt, int x, int y, Location expectedLoc)
    // {
    //     std::unique_ptr<Geometry> geom = r.read(wkt);
    //     RelatePointLocator ael(geom.get());
    //     Coordinate c(x, y);
    //     Location loc = ael.locate(&c);
    //     ensure_equals("Locations are not equal ", expectedLoc, loc);
    // }


    void checkDimLocation(const std::string& wkt, double i, double j, int expected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        RelatePointLocator locator(geom.get());
        CoordinateXY c(i, j);
        int actual = locator.locateWithDim(&c);
        ensure_equals("checkLocation", expected, actual);
    }

    void checkNodeLocation(const std::string& wkt, double i, double j, Location expected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        RelatePointLocator locator(geom.get());
        CoordinateXY c(i, j);
        Location actual = locator.locateNode(&c, nullptr);
        ensure_equals("checkNodeLocation", expected, actual);
    }

};

typedef test_group<test_relatepointlocator_data> group;
typedef group::object object;

group test_relatepointlocator_group("geos::operation::relateng::RelatePointLocator");


// testPoint
template<>
template<>
void object::test<1> ()
{
    //std::string wkt("GEOMETRYCOLLECTION (POINT(0 0), POINT(1 1))");
    checkDimLocation(gcPLA, 1, 1, DimensionLocation::POINT_INTERIOR);
    checkDimLocation(gcPLA, 0, 1, DimensionLocation::EXTERIOR);
}

// testPointInLine
template<>
template<>
void object::test<2> ()
{
    checkDimLocation(gcPLA, 3, 8, DimensionLocation::LINE_INTERIOR);
}

// testPointInArea
template<>
template<>
void object::test<3> ()
{
    checkDimLocation(gcPLA, 8, 8, DimensionLocation::AREA_INTERIOR);
}

// testLine
template<>
template<>
void object::test<4> ()
{
    checkDimLocation(gcPLA, 3, 3, DimensionLocation::LINE_INTERIOR);
    checkDimLocation(gcPLA, 3, 1, DimensionLocation::LINE_BOUNDARY);
}

// testLineInArea
template<>
template<>
void object::test<5> ()
{
    checkDimLocation(gcPLA, 11, 11, DimensionLocation::AREA_INTERIOR);
    checkDimLocation(gcPLA, 14, 14, DimensionLocation::AREA_INTERIOR);
}

// testArea
template<>
template<>
void object::test<6> ()
{
    checkDimLocation(gcPLA, 8, 8, DimensionLocation::AREA_INTERIOR);
    checkDimLocation(gcPLA, 9, 9, DimensionLocation::AREA_BOUNDARY);
}

// testAreaInArea
template<>
template<>
void object::test<7> ()
{
    checkDimLocation(gcPLA, 11, 11, DimensionLocation::AREA_INTERIOR);
    checkDimLocation(gcPLA, 12, 12, DimensionLocation::AREA_INTERIOR);
    checkDimLocation(gcPLA, 10, 10, DimensionLocation::AREA_BOUNDARY);
    checkDimLocation(gcPLA, 16, 16, DimensionLocation::AREA_INTERIOR);
}

// testLineNode
template<>
template<>
void object::test<8> ()
{
    //checkNodeLocation(gcPLA, 12.1, 12.2, Location::INTERIOR);
    checkNodeLocation(gcPLA, 3, 1, Location::BOUNDARY);
}



} // namespace tut
