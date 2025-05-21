//
// Test Suite for geos::coverage::LineDissolver class.

#include <tut/tut.hpp>
#include <utility.h>
#include <string>
#include <memory>

// geos
#include <geos/dissolve/LineDissolver.h>

using geos::dissolve::LineDissolver;

namespace tut {

// Common data used by all tests
struct test_linedissolver_data {

    WKTReader r;
    WKTWriter w;

    test_linedissolver_data() {}

    void checkDissolve(const std::string& wkt, const std::string& wkt_expected)
    {
        std::vector<std::string> wkt_vec;
        wkt_vec.push_back(wkt);
        checkDissolve(wkt_vec, wkt_expected);
    }

    void checkDissolve(std::vector<std::string>& wkt_vec, const std::string& wkt_expected)
    {
        std::vector<std::unique_ptr<Geometry>> geom_store;
        std::vector<const Geometry*> geoms;
        for (const std::string& wkt : wkt_vec) {
            std::unique_ptr<Geometry> geom = r.read(wkt);
            geoms.push_back(geom.get());
            geom_store.emplace_back(geom.release());
        }
        std::unique_ptr<Geometry> geom_expected = r.read(wkt_expected);
        checkDissolve(geoms, geom_expected.get());
    }

    void checkDissolve(
        std::vector<const Geometry*>& geoms,
        const Geometry* geom_expected)
    {
        LineDissolver d;
        d.add(geoms);
        std::unique_ptr<Geometry> result = d.getResult();
        ensure_equals_geometry(geom_expected, result.get());
    }
};


typedef test_group<test_linedissolver_data> group;
typedef group::object object;

group test_linedissolver_data("geos::dissolve::LineDissolver");


// testSingleSegmentLine
template<>
template<>
void object::test<1> ()
{
    checkDissolve(
        "LINESTRING (0 0, 1 1)",
        "LINESTRING (0 0, 1 1)");
}

// testTwoSegmentLine
template<>
template<>
void object::test<2>()
{
    checkDissolve(
        "LINESTRING (0 0, 1 1, 2 2)",
        "LINESTRING (0 0, 1 1, 2 2)");
}

// testOverlappingTwoSegmentLines
template<>
template<>
void object::test<3>()
{
    std::vector<std::string> wkts = {
        "LINESTRING (0 0, 1 1, 2 2)",
        "LINESTRING (1 1, 2 2, 3 3)"};

    checkDissolve(
        wkts,
        "LINESTRING (0 0, 1 1, 2 2, 3 3)");
}

// testOverlappingLines3
template<>
template<>
void object::test<4>()
{
    std::vector<std::string> wkts = {
        "LINESTRING (0 0, 1 1, 2 2)",
        "LINESTRING (1 1, 2 2, 3 3)",
        "LINESTRING (1 1, 2 2, 2 0)" };

    checkDissolve(wkts,
        "MULTILINESTRING ((0 0, 1 1, 2 2), (2 0, 2 2), (2 2, 3 3))");
}

// testDivergingLines
template<>
template<>
void object::test<5>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 1 0, 2 1), (0 0, 1 0, 2 0), (1 0, 2 1, 2 0, 3 0))",
        "MULTILINESTRING ((0 0, 1 0), (1 0, 2 0), (1 0, 2 1, 2 0), (2 0, 3 0))");
}

// testLollipop
template<>
template<>
void object::test<6>()
{
    checkDissolve(
        "LINESTRING (0 0, 1 0, 2 0, 2 1, 1 0, 0 0)",
        "MULTILINESTRING ((0 0, 1 0), (1 0, 2 0, 2 1, 1 0))");
}

// testDisjointLines
template<>
template<>
void object::test<7>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 1 0, 2 1), (10 0, 11 0, 12 0))",
        "MULTILINESTRING ((0 0, 1 0, 2 1), (10 0, 11 0, 12 0))");
}

// testSingleLine
template<>
template<>
void object::test<8>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 1 0, 2 1))",
        "LINESTRING (0 0, 1 0, 2 1)");
}

// testOneSegmentY
template<>
template<>
void object::test<9>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 1 1, 2 2), (1 1, 1 2))",
        "MULTILINESTRING ((0 0, 1 1), (1 1, 2 2), (1 1, 1 2))");
}

// testTwoSegmentY
template<>
template<>
void object::test<10>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 9 9, 10 10, 11 11, 20 20), (10 10, 10 20))",
        "MULTILINESTRING ((10 20, 10 10), (10 10, 9 9, 0 0), (10 10, 11 11, 20 20))");
}

// testIsolatedRing
template<>
template<>
void object::test<11>()
{
    checkDissolve(
        "LINESTRING (0 0, 1 1, 1 0, 0 0)",
        "LINESTRING (0 0, 1 1, 1 0, 0 0)");
}

// testIsolateRingFromMultipleLineStrings
template<>
template<>
void object::test<12>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 1 0, 1 1), (0 0, 0 1, 1 1))",
        "LINESTRING (0 0, 0 1, 1 1, 1 0, 0 0)");
}

/**
* Shows that rings with incident lines are created with the correct node point.
*/
// testRingWithTail
template<>
template<>
void object::test<13>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 1 0, 1 1), (0 0, 0 1, 1 1), (1 0, 2 0))",
        "MULTILINESTRING ((1 0, 0 0, 0 1, 1 1, 1 0), (1 0, 2 0))");
}

// testZeroLengthStartSegment
template<>
template<>
void object::test<14>()
{
    checkDissolve(
        "MULTILINESTRING ((0 0, 0 0, 2 1))",
        "LINESTRING (0 0, 2 1)");
}


} // namespace tut
