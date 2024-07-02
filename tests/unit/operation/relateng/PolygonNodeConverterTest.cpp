//
// Test Suite for geos::operation::relateng::PolygonNodeConverter class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/operation/relateng/PolygonNodeConverter.h>
#include <geos/algorithm/PolygonNodeTopology.h>


// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::algorithm::PolygonNodeTopology;
using geos::io::WKTReader;
// using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_polygonnodeconverter_data {

    WKTReader r;
    // WKTWriter w;

    std::unique_ptr<CoordinateSequence>
    readPts(const std::string& wkt)
    {
        std::unique_ptr<Geometry> line = r.read(wkt);
        return line->getCoordinates();
    }

    void
    checkCrossing(const std::string& wktA, const std::string& wktB)
    {
        checkCrossing(wktA, wktB, true);
    }

    void
    checkNonCrossing(const std::string& wktA, const std::string& wktB)
    {
        checkCrossing(wktA, wktB, false);
    }

    void
    checkCrossing(const std::string& wktA, const std::string& wktB, bool isExpected)
    {
        auto a = readPts(wktA);
        auto b = readPts(wktB);
        // assert: a[1] = b[1]
        bool isCrossing = PolygonNodeTopology::isCrossing(
            &(a->getAt<CoordinateXY>(1)),
            &(a->getAt<CoordinateXY>(0)),
            &(a->getAt<CoordinateXY>(2)),
            &(b->getAt<CoordinateXY>(0)),
            &(b->getAt<CoordinateXY>(2)));
        ensure("checkCrossing", isCrossing == isExpected);
    }

    void
    checkInterior(const std::string& wktA, const std::string& wktB)
    {
        checkInteriorSegment(wktA, wktB, true);
    }

    void
    checkExterior(const std::string& wktA, const std::string& wktB)
    {
        checkInteriorSegment(wktA, wktB, false);
    }

    void
    checkInteriorSegment(const std::string& wktA, const std::string& wktB, bool isExpected)
    {
        auto a = readPts(wktA);
        auto b = readPts(wktB);
        // assert: a[1] = b[1]
        bool isInterior = PolygonNodeTopology::isInteriorSegment(
            &(a->getAt<CoordinateXY>(1)),
            &(a->getAt<CoordinateXY>(0)),
            &(a->getAt<CoordinateXY>(2)),
            &(b->getAt<CoordinateXY>(1)));
        ensure("checkInteriorSegment", isInterior == isExpected);
    }

};

typedef test_group<test_polygonnodeconverter_data> group;
typedef group::object object;

group test_polygonnodeconverter_group("geos::operation::relateng::PolygonNodeConverter");

//
// Test Cases
//


// testNonCrossing
template<>
template<>
void object::test<1> ()
{
    checkCrossing("LINESTRING (500 1000, 1000 1000, 1000 1500)",
        "LINESTRING (1000 500, 1000 1000, 500 1500)");
}

// testNonCrossingQuadrant2
template<>
template<>
void object::test<2> ()
{
    checkNonCrossing("LINESTRING (500 1000, 1000 1000, 1000 1500)",
        "LINESTRING (300 1200, 1000 1000, 500 1500)");
}

// testNonCrossingQuadrant4
template<>
template<>
void object::test<3> ()
{
    checkNonCrossing("LINESTRING (500 1000, 1000 1000, 1000 1500)",
        "LINESTRING (1000 500, 1000 1000, 1500 1000)");
}

// testNonCrossingCollinear
template<>
template<>
void object::test<4> ()
{
    checkNonCrossing("LINESTRING (3 1, 5 5, 9 9)",
        "LINESTRING (2 1, 5 5, 9 9)");
}

// testNonCrossingBothCollinear
template<>
template<>
void object::test<5> ()
{
    checkNonCrossing("LINESTRING (3 1, 5 5, 9 9)",
        "LINESTRING (3 1, 5 5, 9 9)");
}

// testInteriorSegment
template<>
template<>
void object::test<6> ()
{
    checkInterior("LINESTRING (5 9, 5 5, 9 5)",
        "LINESTRING (5 5, 0 0)");
}

// testExteriorSegment
template<>
template<>
void object::test<7> ()
{
    checkExterior("LINESTRING (5 9, 5 5, 9 5)",
        "LINESTRING (5 5, 9 9)");
}


} // namespace tut
