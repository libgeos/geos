//
// Test Suite for geos::operation::coverageunionng::OverlayNG class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/CoverageUnion.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coverageunionng_data {

    WKTReader r;
    WKTWriter w;

    void
    checkUnion(const std::string& wkt, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        std::unique_ptr<Geometry> result = CoverageUnion::geomunion(geom.get());

        try {
            ensure_equals_geometry_xyzm(result.get(), expected.get());
        } catch (const std::exception& e) {
            std::string wkt_result = w.write(result.get());
            std::cerr << std::endl << wkt_result << std::endl;
            throw;
        }
    }

};

typedef test_group<test_coverageunionng_data> group;
typedef group::object object;

group test_coverageunionng_group("geos::operation::overlayng::CoverageUnionNG");

//
// Test Cases
//

// testFilledHole
template<>
template<>
void object::test<1> ()
{
    checkUnion(
        "MULTIPOLYGON (((100 200, 200 200, 200 100, 100 100, 100 200), (120 180, 180 180, 180 120, 120 120, 120 180)), ((180 120, 120 120, 120 180, 180 180, 180 120)))",
        "POLYGON ((200 200, 200 100, 100 100, 100 200, 200 200))"
        );
}

// test3Squares
template<>
template<>
void object::test<2> ()
{
    checkUnion(
        "MULTIPOLYGON (((1 4, 3 4, 3 2, 1 2, 1 4)), ((5 4, 5 2, 3 2, 3 4, 5 4)), ((7 4, 7 2, 5 2, 5 4, 7 4)))",
        "POLYGON ((3 4, 5 4, 7 4, 7 2, 5 2, 3 2, 1 2, 1 4, 3 4))"
        );
}


// testPolygonsSimple
template<>
template<>
void object::test<3> ()
{
    checkUnion(
        "MULTIPOLYGON (((5 5, 1 5, 5 1, 5 5)), ((5 9, 1 5, 5 5, 5 9)), ((9 5, 5 5, 5 9, 9 5)), ((9 5, 5 1, 5 5, 9 5)))",
        "POLYGON ((1 5, 5 9, 9 5, 5 1, 1 5))");
}

// testPolygonsConcentricDonuts
template<>
template<>
void object::test<4> ()
{
    checkUnion(
        "MULTIPOLYGON (((1 9, 9 9, 9 1, 1 1, 1 9), (2 8, 8 8, 8 2, 2 2, 2 8)), ((3 7, 7 7, 7 3, 3 3, 3 7), (4 6, 6 6, 6 4, 4 4, 4 6)))",
        "MULTIPOLYGON (((9 1, 1 1, 1 9, 9 9, 9 1), (8 8, 2 8, 2 2, 8 2, 8 8)), ((7 7, 7 3, 3 3, 3 7, 7 7), (4 4, 6 4, 6 6, 4 6, 4 4)))");
}

// testPolygonsConcentricHalfDonuts
template<>
template<>
void object::test<5> ()
{
    checkUnion(
        "MULTIPOLYGON (((6 9, 1 9, 1 1, 6 1, 6 2, 2 2, 2 8, 6 8, 6 9)), ((6 9, 9 9, 9 1, 6 1, 6 2, 8 2, 8 8, 6 8, 6 9)), ((5 7, 3 7, 3 3, 5 3, 5 4, 4 4, 4 6, 5 6, 5 7)), ((5 4, 5 3, 7 3, 7 7, 5 7, 5 6, 6 6, 6 4, 5 4)))",
        "MULTIPOLYGON (((1 9, 6 9, 9 9, 9 1, 6 1, 1 1, 1 9), (2 8, 2 2, 6 2, 8 2, 8 8, 6 8, 2 8)), ((5 3, 3 3, 3 7, 5 7, 7 7, 7 3, 5 3), (5 4, 6 4, 6 6, 5 6, 4 6, 4 4, 5 4)))");
}

// testPolygonsNested
template<>
template<>
void object::test<6> ()
{
    checkUnion(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9), (3 7, 3 3, 7 3, 7 7, 3 7)), POLYGON ((3 7, 7 7, 7 3, 3 3, 3 7)))",
        "POLYGON ((1 1, 1 9, 9 9, 9 1, 1 1))");
}

// testPolygonsFormingHole
template<>
template<>
void object::test<7> ()
{
    checkUnion(
        "MULTIPOLYGON (((1 1, 4 3, 5 6, 5 9, 1 1)), ((1 1, 9 1, 6 3, 4 3, 1 1)), ((9 1, 5 9, 5 6, 6 3, 9 1)))",
        "POLYGON ((9 1, 1 1, 5 9, 9 1), (6 3, 5 6, 4 3, 6 3))");
}

// testPolygonsSquareGrid
template<>
template<>
void object::test<8> ()
{
    checkUnion(
        "MULTIPOLYGON (((0 0, 0 25, 25 25, 25 0, 0 0)), ((0 25, 0 50, 25 50, 25 25, 0 25)), ((0 50, 0 75, 25 75, 25 50, 0 50)), ((0 75, 0 100, 25 100, 25 75, 0 75)), ((25 0, 25 25, 50 25, 50 0, 25 0)), ((25 25, 25 50, 50 50, 50 25, 25 25)), ((25 50, 25 75, 50 75, 50 50, 25 50)), ((25 75, 25 100, 50 100, 50 75, 25 75)), ((50 0, 50 25, 75 25, 75 0, 50 0)), ((50 25, 50 50, 75 50, 75 25, 50 25)), ((50 50, 50 75, 75 75, 75 50, 50 50)), ((50 75, 50 100, 75 100, 75 75, 50 75)), ((75 0, 75 25, 100 25, 100 0, 75 0)), ((75 25, 75 50, 100 50, 100 25, 75 25)), ((75 50, 75 75, 100 75, 100 50, 75 50)), ((75 75, 75 100, 100 100, 100 75, 75 75)))",
        "POLYGON ((0 25, 0 50, 0 75, 0 100, 25 100, 50 100, 75 100, 100 100, 100 75, 100 50, 100 25, 100 0, 75 0, 50 0, 25 0, 0 0, 0 25))");
}

/**
* Sequential lines are still noded
*/
template<>
template<>
void object::test<9> ()
{
    checkUnion(
        "MULTILINESTRING ((1 1, 5 1), (9 1, 5 1))",
        "MULTILINESTRING ((1 1, 5 1), (5 1, 9 1))");
}

/**
* Overlapping lines are noded with common portions merged
*/
template<>
template<>
void object::test<10> ()
{
    checkUnion(
        "MULTILINESTRING ((1 1, 2 1, 3 1), (4 1, 3 1, 2 1))",
        "MULTILINESTRING ((1 1, 2 1), (2 1, 3 1), (3 1, 4 1))");
}

/**
* A network of lines is dissolved noded at degree > 2 vertices
*/
template<>
template<>
void object::test<11> ()
{
    checkUnion(
        "MULTILINESTRING ((1 9, 3.1 8, 5 7, 7 8, 9 9), (5 7, 5 3, 4 3, 2 3), (9 5, 7 4, 5 3, 8 1))",
        "MULTILINESTRING ((1 9, 3.1 8), (2 3, 4 3), (3.1 8, 5 7), (4 3, 5 3), (5 3, 5 7), (5 3, 7 4), (5 3, 8 1), (5 7, 7 8), (7 4, 9 5), (7 8, 9 9))");
}

// Z values preserved in linear inpuuts
template<>
template<>
void object::test<12> ()
{
    checkUnion(
        "MULTILINESTRING Z ((1 1 8, 5 1 9), (9 1 6, 5 1 2))",
        "MULTILINESTRING Z ((1 1 8, 5 1 9), (5 1 2, 9 1 6))");
}

// M values preserved in linear inpuuts
template<>
template<>
void object::test<13> ()
{
    checkUnion(
        "MULTILINESTRING M ((1 1 8, 5 1 9), (9 1 6, 5 1 2))",
        "MULTILINESTRING M ((1 1 8, 5 1 9), (5 1 2, 9 1 6))");
}

// Mixed Z/M values handled in linear inputs
// missing Z values are populated by ElevationModel
template<>
template<>
void object::test<14>()
{
    checkUnion("GEOMETRYCOLLECTION (LINESTRING Z(1 1 8, 5 1 9), LINESTRING M(9 1 6, 5 1 2))",
               "MULTILINESTRING ZM ((1 1 8 NaN, 5 1 9 NaN), (5 1 9 2, 9 1 8.5 6))");
}

// Z values preserved in polygonal inputs
template<>
template<>
void object::test<15>()
{
    checkUnion("GEOMETRYCOLLECTION( POLYGON Z ((0 0 0, 1 0 1, 1 1 2, 0 0 0)), POLYGON Z ((0 0 0, 1 1 2, 0 1 3, 0 0 0)) )",
               "POLYGON Z ((0 0 0, 1 0 1, 1 1 2, 0 1 3, 0 0 0))");
}

// M values preserved in polygonal inputs
template<>
template<>
void object::test<17>()
{
    checkUnion("GEOMETRYCOLLECTION( POLYGON M ((0 0 0, 1 0 1, 1 1 2, 0 0 0)), POLYGON M ((0 0 0, 1 1 2, 0 1 3, 0 0 0)) )",
               "POLYGON M ((0 0 0, 1 0 1, 1 1 2, 0 1 3, 0 0 0))");
}

} // namespace tut
