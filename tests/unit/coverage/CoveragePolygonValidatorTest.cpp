//
// Test Suite for geos::coverage::CoveragePolygonValidatorTest class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/CoveragePolygonValidator.h>
#include <geos/geom/util/PolygonExtracter.h>

using geos::geom::util::PolygonExtracter;
using geos::coverage::CoveragePolygonValidator;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coveragepolygonvalidator_data {

    WKTReader r;
    WKTWriter w;

    void
    printResult(std::unique_ptr<Geometry>& actual, std::unique_ptr<Geometry>& expected)
    {
        std::cout << std::endl;
        std::cout << "--actual--" << std::endl;
        std::cout << w.write(actual.get()) << std::endl;
        std::cout << "--expect--" << std::endl;
        std::cout << w.write(expected.get()) << std::endl;
    }

    void
    checkInvalid(const std::string& wktTarget, const std::string& wktAdj, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> target = r.read(wktTarget);
        std::unique_ptr<Geometry> adj = r.read(wktAdj);
        std::vector<const Geometry*> adjPolygons = extractPolygons(adj);
        std::unique_ptr<Geometry> actual = CoveragePolygonValidator::validate(target.get(), adjPolygons);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        // printResult(actual, expected);
        ensure_equals_geometry(expected.get(), actual.get());
    }

    void
    checkInvalidGap(const std::string& wktTarget, const std::string& wktAdj,
        double gapWidth, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> target = r.read(wktTarget);
        std::unique_ptr<Geometry> adj = r.read(wktAdj);
        std::vector<const Geometry*> adjPolygons = extractPolygons(adj);
        std::unique_ptr<Geometry> actual = CoveragePolygonValidator::validate(target.get(), adjPolygons, gapWidth);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        // printResult(actual, expected);
        ensure_equals_geometry(expected.get(), actual.get());
    }

    void
    checkValid(const std::string& wktTarget, const std::string& wktAdj)
    {
        std::unique_ptr<Geometry> target = r.read(wktTarget);
        std::unique_ptr<Geometry> adj = r.read(wktAdj);
        std::vector<const Geometry*> adjPolygons = extractPolygons(adj);
        std::unique_ptr<Geometry> actual = CoveragePolygonValidator::validate(target.get(), adjPolygons);
        std::unique_ptr<Geometry> expected = r.read("LINESTRING EMPTY");
        // printResult(actual, expected);
        ensure_equals_geometry(expected.get(), actual.get());
    }

    std::vector<const Geometry*>
    extractPolygons(const std::unique_ptr<Geometry>& geom)
    {
        std::vector<const Polygon*> polygons;
        PolygonExtracter::getPolygons(*geom, polygons);
        std::vector<const Geometry*> geoms;
        for (const Polygon* poly: polygons) {
            geoms.push_back(static_cast<const Geometry*>(poly));
        }
        return geoms;
    }

};


typedef test_group<test_coveragepolygonvalidator_data> group;
typedef group::object object;

group test_coveragepolygonvalidator_data("geos::coverage::CoveragePolygonValidator");


//========  Invalid cases   =============================

// testCollinearUnmatchedEdge
template<>
template<>
void object::test<1> ()
{
    checkInvalid("POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))",
        "POLYGON ((100 300, 180 300, 180 200, 100 200, 100 300))",
        "LINESTRING (100 200, 200 200)");
}

// testDuplicate
template<>
template<>
void object::test<2> ()
{
    checkInvalid("POLYGON ((1 3, 3 3, 3 1, 1 1, 1 3))",
        "MULTIPOLYGON (((1 3, 3 3, 3 1, 1 1, 1 3)), ((5 3, 5 1, 3 1, 3 3, 5 3)))",
        "LINESTRING (1 3, 3 3, 3 1, 1 1, 1 3)");
}

// testDuplicateReversed
template<>
template<>
void object::test<3> ()
{
    checkInvalid("POLYGON ((1 3, 3 3, 3 1, 1 1, 1 3))",
        "MULTIPOLYGON (((1 3, 1 1, 3 1, 3 3, 1 3)), ((5 3, 5 1, 3 1, 3 3, 5 3)))",
        "LINESTRING (1 3, 1 1, 3 1, 3 3, 1 3)");
}

// testCrossingSegment
template<>
template<>
void object::test<4> ()
{
    checkInvalid("POLYGON ((1 9, 9 9, 9 3, 1 3, 1 9))",
        "POLYGON ((1 1, 5 6, 9 1, 1 1))",
        "LINESTRING (1 3, 9 3)");
}

// testCrossingAndInteriorSegments
template<>
template<>
void object::test<5> ()
{
    checkInvalid("POLYGON ((1 1, 3 4, 7 4, 9 1, 1 1))",
        "POLYGON ((1 9, 9 9, 9 3, 1 3, 1 9))",
        "LINESTRING (1 1, 3 4, 7 4, 9 1)");
}

// testTargetVertexTouchesSegment
template<>
template<>
void object::test<6> ()
{
    checkInvalid("POLYGON ((1 9, 9 9, 9 5, 1 5, 1 9))",
        "POLYGON ((1 1, 5 5, 9 1, 1 1))",
        "LINESTRING (9 5, 1 5)");
}

// testAdjVertexTouchesSegment
template<>
template<>
void object::test<7> ()
{
    checkInvalid("POLYGON ((1 1, 5 5, 9 1, 1 1))",
        "POLYGON ((1 9, 9 9, 9 5, 1 5, 1 9))",
        "LINESTRING (1 1, 5 5, 9 1)");
}

// testInteriorSegmentTouchingEdge
template<>
template<>
void object::test<8> ()
{
    checkInvalid("POLYGON ((4 3, 4 7, 8 9, 8 1, 4 3))",
        "POLYGON ((1 7, 6 7, 6 3, 1 3, 1 7))",
        "LINESTRING (8 1, 4 3, 4 7, 8 9)");
}

// testInteriorSegmentTouchingNodes
template<>
template<>
void object::test<9> ()
{
    checkInvalid("POLYGON ((4 2, 4 8, 8 9, 8 1, 4 2))",
        "POLYGON ((1 5, 4 8, 7 5, 4 2, 1 5))",
        "LINESTRING (4 2, 4 8)");
}

// testInteriorSegmentsTouching
template<>
template<>
void object::test<10> ()
{
    checkInvalid("POLYGON ((1 9, 5 9, 8 7, 5 7, 3 5, 8 2, 1 2, 1 9))",
        "POLYGON ((5 9, 9 9, 9 1, 5 1, 5 9))",
        "LINESTRING (5 9, 8 7, 5 7, 3 5, 8 2, 1 2)");
}

// testTargetMultiPolygon
template<>
template<>
void object::test<11> ()
{
    checkInvalid("MULTIPOLYGON (((4 8, 9 9, 9 7, 4 8)), ((3 5, 9 6, 9 4, 3 5)), ((2 2, 9 3, 9 1, 2 2)))",
        "POLYGON ((1 1, 1 9, 5 9, 6 7, 5 5, 6 3, 5 1, 1 1))",
        "MULTILINESTRING ((9 7, 4 8, 9 9), (9 4, 3 5, 9 6), (9 1, 2 2, 9 3))");
}

// testBothMultiPolygon
template<>
template<>
void object::test<12> ()
{
    checkInvalid("MULTIPOLYGON (((4 8, 9 9, 9 7, 4 8)), ((3 5, 9 6, 9 4, 3 5)), ((2 2, 9 3, 9 1, 2 2)))",
        "MULTIPOLYGON (((1 6, 1 9, 5 9, 6 7, 5 5, 1 6)), ((1 4, 5 5, 6 3, 5 1, 1 1, 1 4)))",
        "MULTILINESTRING ((9 7, 4 8, 9 9), (9 4, 3 5, 9 6), (9 1, 2 2, 9 3))");
}

/**
* Shows need to evaluate both start and end point of intersecting segments
* in InvalidSegmentDetector,
* since matched segments are not tested
*/
// testInteriorSegmentsWithMatch
template<>
template<>
void object::test<13> ()
{
    checkInvalid("POLYGON ((7 6, 1 1, 3 6, 7 6))",
        "MULTIPOLYGON (((1 9, 9 9, 9 1, 1 1, 3 6, 1 9)), ((0 1, 0 9, 1 9, 3 6, 1 1, 0 1)))",
        "LINESTRING (7 6, 1 1, 3 6, 7 6)");
}

// testAdjacentHoleOverlap
template<>
template<>
void object::test<14> ()
{
    checkInvalid("POLYGON ((3 3, 3 7, 6 8, 7 3, 3 3))",
        "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9), (3 7, 7 7, 7 3, 3 3, 3 7))",
        "LINESTRING (3 7, 6 8, 7 3)");
}

// testTargetHoleOverlap
template<>
template<>
void object::test<15> ()
{
    checkInvalid("POLYGON ((1 1, 1 9, 9 9, 9 1, 1 1), (2 2, 8 2, 8 8, 5 4, 3 5, 2 5, 2 2))",
        "POLYGON ((2 2, 2 5, 3 5, 8 6.7, 8 2, 2 2))",
        "LINESTRING (8 2, 8 8, 5 4, 3 5)");
}

// testFullyContained
template<>
template<>
void object::test<16> ()
{
    checkInvalid("POLYGON ((3 7, 7 7, 7 3, 3 3, 3 7))",
        "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
        "LINESTRING (3 7, 7 7, 7 3, 3 3, 3 7)");
}

// testFullyCoveredAndMatched
template<>
template<>
void object::test<17> ()
{
    checkInvalid("POLYGON ((1 3, 2 3, 2 2, 1 2, 1 3))",
        "MULTIPOLYGON (((1 1, 1 2, 2 2, 2 1, 1 1)), ((3 1, 2 1, 2 2, 3 2, 3 1)), ((3 3, 3 2, 2 2, 2 3, 3 3)), ((2 3, 3 3, 3 2, 3 1, 2 1, 1 1, 1 2, 1 3, 2 3)))",
        "LINESTRING (1 2, 1 3, 2 3)");
}

// testTargetCoveredAndMatching
template<>
template<>
void object::test<18> ()
{
    checkInvalid("POLYGON ((1 7, 5 7, 9 7, 9 3, 5 3, 1 3, 1 7))",
        "MULTIPOLYGON (((5 9, 9 7, 5 7, 1 7, 5 9)), ((1 7, 5 7, 5 3, 1 3, 1 7)), ((9 3, 5 3, 5 7, 9 7, 9 3)), ((1 3, 5 3, 9 3, 5 1, 1 3)))",
        "LINESTRING (1 7, 5 7, 9 7, 9 3, 5 3, 1 3, 1 7)");
}

// testCoveredBy2AndMatching
template<>
template<>
void object::test<19> ()
{
    checkInvalid("POLYGON ((1 9, 9 9, 9 5, 1 5, 1 9))",
        "MULTIPOLYGON (((1 5, 9 5, 9 1, 1 1, 1 5)), ((1 9, 5 9, 5 1, 1 1, 1 9)), ((9 9, 9 1, 5 1, 5 9, 9 9)))",
        "LINESTRING (1 5, 1 9, 9 9, 9 5)");
}



//========  Gap cases   =============================

// testGap
template<>
template<>
void object::test<20> ()
{
    checkInvalidGap("POLYGON ((1 5, 9 5, 9 1, 1 1, 1 5))",
        "POLYGON ((1 9, 5 9, 5 5.1, 1 5, 1 9))",
        0.5,
        "LINESTRING (1 5, 9 5)");
}

//========  Valid cases   =============================

// testMatchedEdges
template<>
template<>
void object::test<21> ()
{
    checkValid("POLYGON ((3 7, 7 7, 7 3, 3 3, 3 7))",
        "MULTIPOLYGON (((1 7, 3 7, 3 3, 1 3, 1 7)), ((3 9, 7 9, 7 7, 3 7, 3 9)), ((9 7, 9 3, 7 3, 7 7, 9 7)), ((3 1, 3 3, 7 3, 7 1, 3 1)))");
}

// testRingsCCW
template<>
template<>
void object::test<22> ()
{
    checkValid("POLYGON ((1 1, 6 5, 4 9, 1 9, 1 1))",
        "POLYGON ((1 1, 9 1, 9 4, 6 5, 1 1))");
}

//-- confirms zero-length segments are skipped in processing
// testRepeatedCommonVertexInTarget
template<>
template<>
void object::test<23> ()
{
    checkValid("POLYGON ((1 1, 1 3, 5 3, 5 3, 9 1, 1 1))",
        "POLYGON ((1 9, 9 9, 9 5, 5 3, 1 3, 1 9))");
}

  //-- confirms zero-length segments are skipped in processing
// testRepeatedCommonVertexInAdjacent
template<>
template<>
void object::test<24> ()
{
    checkValid("POLYGON ((1 1, 1 3, 5 3, 9 1, 1 1))",
        "POLYGON ((1 9, 9 9, 9 5, 5 3, 5 3, 1 3, 1 9))");
}




} // namespace tut
