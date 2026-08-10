//
// Test Suite for geos::algorithm::PointLocation
// Ported from JTS junit/algorithm/PointLocationTest.java

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/geom/Coordinate.h>
#include <geos/algorithm/PointLocation.h>

// std
#include <string>
#include <memory>

using geos::algorithm::PointLocation;
using geos::geom::CoordinateXY;

namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_PointLocation_data {

    geos::io::WKTReader r_;

    void
    checkOnLine(double x, double y, const std::string& wktLine, bool isExpected)
    {
        CoordinateXY p(x, y);
        std::unique_ptr<CoordinateSequence> line = readPts(wktLine);
        bool isOnLine = PointLocation::isOnLine(p, line.get());
        ensure(isOnLine == isExpected);
    }

    void
    checkOnSegment(double x, double y, const std::string& wktLine, bool isExpected)
    {
        CoordinateXY p(x, y);
        std::unique_ptr<CoordinateSequence> line = readPts(wktLine);

        bool isOnSeg = PointLocation::isOnSegment(p, line->getAt(0), line->getAt(1));
        ensure(isOnSeg == isExpected);
    }

    std::unique_ptr<CoordinateSequence>
    readPts(const std::string& wkt)
    {
        auto line = r_.read<LineString>(wkt);
        if (line)
            return line->getCoordinatesRO()->clone();
        else
            return nullptr;
    }

};


typedef test_group<test_PointLocation_data> group;
typedef group::object object;

group test_PointLocation_data("geos::algorithm::PointLocation");


//
// Test Cases
//

// testOnLineOnVertex
template<>
template<>
void object::test<1> ()
{
    checkOnLine(20, 20, "LINESTRING (0 00, 20 20, 30 30)", true);
}

// testOnLineInSegment
template<>
template<>
void object::test<2> ()
{
    checkOnLine(10, 10, "LINESTRING (0 0, 20 20, 0 40)", true);
    checkOnLine(10, 30, "LINESTRING (0 0, 20 20, 0 40)", true);
}

// testNotOnLine
template<>
template<>
void object::test<3> ()
{
    checkOnLine(0, 100, "LINESTRING (10 10, 20 10, 30 10)", false);
}

// testOnSegment
template<>
template<>
void object::test<4> ()
{
    checkOnSegment(5, 5, "LINESTRING(0 0, 9 9)", true);
    checkOnSegment(0, 0, "LINESTRING(0 0, 9 9)", true);
    checkOnSegment(9, 9, "LINESTRING(0 0, 9 9)", true);
}

// testNotOnSegment
template<>
template<>
void object::test<5> ()
{
    checkOnSegment(5, 6, "LINESTRING(0 0, 9 9)", false);
    checkOnSegment(10, 10, "LINESTRING(0 0, 9 9)", false);
    checkOnSegment(9, 9.00001, "LINESTRING(0 0, 9 9)", false);
}

// testOnZeroLengthSegment
template<>
template<>
void object::test<6> ()
{
    checkOnSegment(1, 1, "LINESTRING(1 1, 1 1)", true);
    checkOnSegment(1, 2, "LINESTRING(1 1, 1 1)", false);
}

// testOnSegmentFloatCollinear — GEOS #968 / PostGIS #5563
// Line (1,0)-(0,2) is parametrically (1-t, 2t). Points at t=0.1 and t=0.9
// are collinear in reals but their binary64 encodings can fail DD orientation.
template<>
template<>
void object::test<7> ()
{
    checkOnSegment(0.9, 0.2, "LINESTRING (1 0, 0 2)", true);
    checkOnSegment(0.1, 1.8, "LINESTRING (1 0, 0 2)", true);
    checkOnSegment(0.5, 1.0, "LINESTRING (1 0, 0 2)", true);
    // clearly off-line still rejected
    checkOnSegment(0.5, 0.5, "LINESTRING (1 0, 0 2)", false);
}


} // namespace tut

