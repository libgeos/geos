//
// Test Suite for geos::coverage::CoverageSimplifierTest class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/CoverageSimplifier.h>

using geos::coverage::CoverageSimplifier;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coveragesimplifier_data {

    WKTReader r;
    WKTWriter w;
    std::vector<std::unique_ptr<Geometry>> geomArray;

    test_coveragesimplifier_data() {
    }

    void checkNoop(
        const std::vector<std::unique_ptr<Geometry>>& input)
    {
        std::vector<std::unique_ptr<Geometry>> actual = CoverageSimplifier::simplify(input, 0);

        // std::cout << w.write(*input[0]) << std::endl;
        // std::cout << w.write(*input[1]) << std::endl;
        // std::cout << "--" << std::endl;
        // std::cout << w.write(*actual[0]) << std::endl;
        // std::cout << w.write(*actual[1]) << std::endl;

        checkArrayEqual(input, actual);
    }

    void checkResult(
        const std::vector<std::unique_ptr<Geometry>>& input,
        double tolerance,
        const std::vector<std::unique_ptr<Geometry>>& expected)
    {
        std::vector<std::unique_ptr<Geometry>> actual = CoverageSimplifier::simplify(input, tolerance);
        checkArrayEqual(expected, actual);
    }

    void checkResultInner(
        const std::vector<std::unique_ptr<Geometry>>& input,
        double tolerance,
        const std::vector<std::unique_ptr<Geometry>>& expected)
    {
        std::vector<std::unique_ptr<Geometry>> actual = CoverageSimplifier::simplifyInner(input, tolerance);
        checkArrayEqual(expected, actual);
    }

    std::vector<std::unique_ptr<Geometry>>
    readArray(std::vector<std::string> wkts)
    {
        std::vector<std::unique_ptr<Geometry>> geometries;
        for (std::string& wkt : wkts) {
            auto geom = r.read(wkt);
            if (geom != nullptr) {
                geometries.push_back(std::move(geom));
            }
        }
        return geometries;
    }

    void
    checkArrayEqual(
        const std::vector<std::unique_ptr<Geometry>>& input,
        const std::vector<std::unique_ptr<Geometry>>& expected)
    {
        ensure("arrays same size", input.size() == expected.size());
        for (std::size_t i = 0; i < input.size(); i++) {
            ensure_equals_geometry(input[i].get(), expected[i].get());
        }
    }

};


typedef test_group<test_coveragesimplifier_data> group;
typedef group::object object;

group test_coveragesimplifier_data("geos::coverage::CoverageSimplifier");


// testNoopSimple2
template<>
template<>
void object::test<1> ()
{
    checkNoop(readArray({
        "POLYGON ((100 100, 200 200, 300 100, 200 101, 100 100))",
        "POLYGON ((150 0, 100 100, 200 101, 300 100, 250 0, 150 0))" })
    );
}

// testNoopSimple3
template<>
template<>
void object::test<2> ()
{
    checkNoop(readArray({
        "POLYGON ((100 300, 200 200, 100 200, 100 300))",
        "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))",
        "POLYGON ((100 100, 200 100, 150 50, 100 100))" })
    );
}

// testNoopHole
template<>
template<>
void object::test<3> ()
{
    checkNoop(readArray({
        "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (20 80, 80 80, 80 20, 20 20, 20 80))",
        "POLYGON ((80 20, 20 20, 20 80, 80 80, 80 20))" })
    );
}

// testNoopMulti
template<>
template<>
void object::test<4> ()
{
    checkNoop(readArray({
        "MULTIPOLYGON (((10 10, 10 50, 50 50, 50 10, 10 10)), ((90 90, 90 50, 50 50, 50 90, 90 90)))",
        "MULTIPOLYGON (((10 90, 50 90, 50 50, 10 50, 10 90)), ((90 10, 50 10, 50 50, 90 50, 90 10)))" })
    );
}

//---------------------------------------------

// testSimple2
template<>
template<>
void object::test<5> ()
{
    checkResult(readArray({
        "POLYGON ((100 100, 200 200, 300 100, 200 101, 100 100))",
        "POLYGON ((150 0, 100 100, 200 101, 300 100, 250 0, 150 0))" }),
        10,
        readArray({
            "POLYGON ((100 100, 200 200, 300 100, 100 100))",
            "POLYGON ((150 0, 100 100, 300 100, 250 0, 150 0))" })
    );
}

// testSingleRingNoCollapse
template<>
template<>
void object::test<6> ()
{
    checkResult(readArray({
        "POLYGON ((10 50, 60 90, 70 50, 60 10, 10 50))" }),
        100000,
        readArray({
            "POLYGON ((10 50, 60 90, 60 10, 10 50))" })
    );
}

/**
* Checks that a polygon on the edge of the coverage does not collapse
* under maximal simplification
*/
// testMultiEdgeRingNoCollapse
template<>
template<>
void object::test<7> ()
{
    checkResult(readArray({
        "POLYGON ((50 250, 200 200, 180 170, 200 150, 50 50, 50 250))",
        "POLYGON ((200 200, 180 170, 200 150, 200 200))" }),
        40,
        readArray({
            "POLYGON ((50 250, 200 200, 180 170, 200 150, 50 50, 50 250))",
            "POLYGON ((200 200, 180 170, 200 150, 200 200))" })
    );
}

// testFilledHole
template<>
template<>
void object::test<8> ()
{
    checkResult(readArray({
            "POLYGON ((20 30, 20 80, 60 50, 80 20, 50 20, 20 30))",
            "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (50 20, 20 30, 20 80, 60 50, 80 20, 50 20))"
        }),
        28,
        readArray({
            "POLYGON ((20 30, 20 80, 80 20, 20 30))",
            "POLYGON ((10 10, 10 90, 90 90, 90 10, 10 10), (20 30, 80 20, 20 80, 20 30))"
        })
    );
}

// testTouchingHoles
template<>
template<>
void object::test<9> ()
{
    checkResult(readArray({
            "POLYGON (( 0 0, 0 11, 19 11, 19 0, 0 0 ), ( 4 5, 12 5, 12 6, 10 6, 10 8, 9 8, 9 9, 7 9, 7 8, 6 8, 6 6, 4 6, 4 5 ), ( 12 6, 14 6, 14 9, 13 9, 13 7, 12 7, 12 6 ))",
            "POLYGON (( 12 6, 12 5, 4 5, 4 6, 6 6, 6 8, 7 8, 7 9, 9 9, 9 8, 10 8, 10 6, 12 6 ))",
            "POLYGON (( 12 6, 12 7, 13 7, 13 9, 14 9, 14 6, 12 6 ))" }),
        1.0,
        readArray({
            "POLYGON ((0 0, 0 11, 19 11, 19 0, 0 0), (4 5, 12 5, 12 6, 10 6, 9 9, 6 8, 6 6, 4 5), (12 6, 14 6, 14 9, 12 6))",
            "POLYGON ((4 5, 6 6, 6 8, 9 9, 10 6, 12 6, 12 5, 4 5))",
            "POLYGON ((12 6, 14 9, 14 6, 12 6))"
        })
    );
}

// testHoleTouchingShell
template<>
template<>
void object::test<10> ()
{
    checkResultInner(readArray({
            "POLYGON ((200 300, 300 300, 300 100, 100 100, 100 300, 200 300), (170 220, 170 160, 200 140, 200 250, 170 220), (170 250, 200 250, 200 300, 170 250))",
            "POLYGON ((170 220, 200 250, 200 140, 170 160, 170 220))",
            "POLYGON ((170 250, 200 300, 200 250, 170 250))" }),
        100.0,
        readArray({
            "POLYGON ((100 100, 100 300, 200 300, 300 300, 300 100, 100 100), (170 160, 200 140, 200 250, 170 160), (170 250, 200 250, 200 300, 170 250))",
            "POLYGON ((170 160, 200 250, 200 140, 170 160))",
            "POLYGON ((200 250, 200 300, 170 250, 200 250))" })
    );
}

// testHolesTouchingHolesAndShellInner
template<>
template<>
void object::test<11> ()
{
    checkResultInner(readArray({
            "POLYGON (( 8 5, 9 4, 9 2, 1 2, 1 4, 2 4, 2 5, 1 5, 1 8, 9 8, 9 6, 8 5 ), ( 8 5, 7 6, 6 6, 6 4, 7 4, 8 5 ), ( 7 6, 8 6, 7 7, 7 6 ), ( 6 6, 6 7, 5 6, 6 6 ), ( 6 4, 5 4, 6 3, 6 4 ), ( 7 4, 7 3, 8 4, 7 4 ))" }),
        4.0,
        readArray({
            "POLYGON (( 8 5, 9 4, 9 2, 1 2, 1 4, 2 4, 2 5, 1 5, 1 8, 9 8, 9 6, 8 5 ), ( 8 5, 7 6, 6 6, 6 4, 7 4, 8 5 ), ( 7 6, 8 6, 7 7, 7 6 ), ( 6 6, 6 7, 5 6, 6 6 ), ( 6 4, 5 4, 6 3, 6 4 ), ( 7 4, 7 3, 8 4, 7 4 ))" })
    );
}

// testHolesTouchingHolesAndShell
template<>
template<>
void object::test<12> ()
{
    checkResult(readArray({
            "POLYGON (( 8 5, 9 4, 9 2, 1 2, 1 4, 2 4, 2 5, 1 5, 1 8, 9 8, 9 6, 8 5 ), ( 8 5, 7 6, 6 6, 6 4, 7 4, 8 5 ), ( 7 6, 8 6, 7 7, 7 6 ), ( 6 6, 6 7, 5 6, 6 6 ), ( 6 4, 5 4, 6 3, 6 4 ), ( 7 4, 7 3, 8 4, 7 4 ))" }),
        4.0,
        readArray({
            "POLYGON (( 1 2, 1 8, 9 8, 8 5, 9 2, 1 2 ), ( 5 4, 6 3, 6 4, 5 4 ), ( 5 6, 6 6, 6 7, 5 6 ), ( 6 4, 7 4, 8 5, 7 6, 6 6, 6 4 ), ( 7 3, 8 4, 7 4, 7 3 ), ( 7 6, 8 6, 7 7, 7 6 ))" })
    );
}

// testMultiPolygonWithTouchingShellsInner
template<>
template<>
void object::test<13> ()
{
    checkResultInner(
        readArray({
            "MULTIPOLYGON ((( 2 7, 2 8, 3 8, 3 7, 2 7 )), (( 1 6, 1 7, 2 7, 2 6, 1 6 )), (( 0 7, 0 8, 1 8, 1 7, 0 7 )), (( 0 5, 0 6, 1 6, 1 5, 0 5 )), (( 2 5, 2 6, 3 6, 3 5, 2 5 )))"
        }),
        1.0,
        readArray({
            "MULTIPOLYGON ((( 2 7, 2 8, 3 8, 3 7, 2 7 )), (( 1 6, 1 7, 2 7, 2 6, 1 6 )), (( 0 7, 0 8, 1 8, 1 7, 0 7 )), (( 0 5, 0 6, 1 6, 1 5, 0 5 )), (( 2 5, 2 6, 3 6, 3 5, 2 5 )))"
        })
    );
}

// testMultiPolygonWithTouchingShells
template<>
template<>
void object::test<14> ()
{
    checkResult(
        readArray({
            "MULTIPOLYGON ((( 2 7, 2 8, 3 8, 3 7, 2 7 )), (( 1 6, 1 7, 2 7, 2 6, 1 6 )), (( 0 7, 0 8, 1 8, 1 7, 0 7 )), (( 0 5, 0 6, 1 6, 1 5, 0 5 )), (( 2 5, 2 6, 3 6, 3 5, 2 5 )))" }),
        1.0,
        readArray({
            "MULTIPOLYGON ((( 2 7, 3 8, 3 7, 2 7 )), (( 1 6, 1 7, 2 7, 2 6, 1 6 )), (( 1 7, 0 8, 1 8, 1 7 )), (( 1 6, 0 5, 0 6, 1 6 )), (( 2 6, 3 5, 2 5, 2 6 )))" })
    );
}

// testTouchingShellsInner
template<>
template<>
void object::test<15> ()
{
    checkResultInner(readArray({
            "POLYGON ((0 0, 0 5, 5 6, 10 5, 10 0, 0 0))",
            "POLYGON ((0 10, 5 6, 10 10, 0 10))" }),
        4.0,
        readArray({
            "POLYGON ((0 0, 0 5, 5 6, 10 5, 10 0, 0 0))",
            "POLYGON ((0 10, 5 6, 10 10, 0 10))" })
    );
}

// testShellSimplificationAtStartingNode
template<>
template<>
void object::test<16> ()
{
    checkResult(readArray({
            "POLYGON (( 1 5, 1 7, 5 7, 5 3, 2 3, 1 5 ))" }),
        1.5,
        readArray({
            "POLYGON ((1 7, 5 7, 5 3, 2 3, 1 7))" })
    );
}

// testSimplifyInnerAtStartingNode
template<>
template<>
void object::test<17> ()
{
    checkResultInner(readArray({
        "POLYGON (( 0 5, 0 9, 6 9, 6 2, 1 2, 0 5 ), ( 1 5, 2 3, 5 3, 5 7, 1 7, 1 5 ))",
            "POLYGON (( 1 5, 1 7, 5 7, 5 3, 2 3, 1 5 ))" }),
        1.5,
        readArray({
            "POLYGON ((0 5, 0 9, 6 9, 6 2, 1 2, 0 5), (1 7, 2 3, 5 3, 5 7, 1 7))",
            "POLYGON ((1 7, 5 7, 5 3, 2 3, 1 7))" })
    );
}

// testSimplifyAllAtStartingNode
template<>
template<>
void object::test<18> ()
{
    checkResult(readArray({
            "POLYGON (( 0 5, 0 9, 6 9, 6 2, 1 2, 0 5 ), ( 1 5, 2 3, 5 3, 5 7, 1 7, 1 5 ))",
            "POLYGON (( 1 5, 1 7, 5 7, 5 3, 2 3, 1 5 ))" }),
        1.5,
        readArray({
            "POLYGON ((0 9, 6 9, 6 2, 1 2, 0 9), (1 7, 2 3, 5 3, 5 7, 1 7))",
            "POLYGON ((1 7, 5 7, 5 3, 2 3, 1 7))" })
    );
}

//---------------------------------
// testInnerSimple
template<>
template<>
void object::test<19> ()
{
    checkResultInner(readArray({
        "POLYGON ((50 50, 50 150, 100 190, 100 200, 200 200, 160 150, 120 120, 90 80, 50 50))",
        "POLYGON ((100 0, 50 50, 90 80, 120 120, 160 150, 200 200, 250 100, 170 50, 100 0))" }),
        100,
        readArray({
            "POLYGON ((50 50, 50 150, 100 190, 100 200, 200 200, 50 50))",
            "POLYGON ((200 200, 50 50, 100 0, 170 50, 250 100, 200 200))" })
    );

}

// testRepeatedPointRemoved
template<>
template<>
void object::test<20> ()
{
    checkResult(readArray({
        "POLYGON ((5 9, 6.5 6.5, 9 5, 5 5, 5 5, 5 9))" }),
        2,
        readArray({
            "POLYGON ((5 5, 5 9, 9 5, 5 5))" })
    );
}

// testRepeatedPointCollapseToLine
template<>
template<>
void object::test<21> ()
{
checkResult(readArray({
    "MULTIPOLYGON (((10 10, 10 20, 20 19, 30 20, 30 10, 10 10)), ((10 30, 20 29, 30 30, 30 20, 20 19, 10 20, 10 30)), ((10 20, 20 19, 20 19, 10 20)))" }),
    5,
    readArray({
        "MULTIPOLYGON (((10 20, 20 19, 30 20, 30 10, 10 10, 10 20)), ((30 20, 20 19, 10 20, 10 30, 30 30, 30 20)), ((10 20, 20 19, 10 20)))" })
);
}

// testRepeatedPointCollapseToPoint()
template<>
template<>
void object::test<22> ()
{
    checkResult(readArray({
        "MULTIPOLYGON (((10 10, 10 20, 20 19, 30 20, 30 10, 10 10)), ((10 30, 20 29, 30 30, 30 20, 20 19, 10 20, 10 30)), ((20 19, 20 19, 20 19)))" }),
        5,
        readArray({
            "MULTIPOLYGON (((10 10, 10 20, 20 19, 30 20, 30 10, 10 10)), ((10 20, 10 30, 30 30, 30 20, 20 19, 10 20)), ((20 19, 20 19, 20 19)))" })
    );
}

// testRepeatedPointCollapseToPoint2
template<>
template<>
void object::test<23> ()
{
    checkResult(readArray({
        "MULTIPOLYGON (((100 200, 150 195, 200 200, 200 100, 100 100, 100 200)), ((150 195, 150 195, 150 195, 150 195)))" }),
        40,
        readArray({
            "MULTIPOLYGON (((150 195, 200 200, 200 100, 100 100, 100 200, 150 195)), ((150 195, 150 195, 150 195, 150 195)))" })
    );
}

// testAllEmpty()
template<>
template<>
void object::test<24> ()
{
    checkResult(readArray({
        "POLYGON EMPTY",
        "POLYGON EMPTY" }),
        1,
        readArray({
            "POLYGON EMPTY",
            "POLYGON EMPTY" })
    );
  }

// testOneEmpty
template<>
template<>
void object::test<25> ()
{
    checkResult(readArray({
        "POLYGON ((1 9, 5 9.1, 9 9, 9 1, 1 1, 1 9))",
        "POLYGON EMPTY" }),
        1,
        readArray({
            "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
            "POLYGON EMPTY" })
    );
  }

// testEmptyHole()
template<>
template<>
void object::test<26> ()
{
    checkResult(readArray({
        "POLYGON ((1 9, 5 9.1, 9 9, 9 1, 1 1, 1 9), EMPTY)",
        "POLYGON EMPTY"  }),
        1,
        readArray({
            "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9), EMPTY)",
            "POLYGON EMPTY" })
    );
  }

} // namespace tut
