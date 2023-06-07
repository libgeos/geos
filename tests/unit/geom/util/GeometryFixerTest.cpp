//
// Test Suite for geos::geom::util::GeometryFixer class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/geom/util/GeometryFixer.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/constants.h>

#include <utility.h>

// std
#include <vector>
#include <limits>

namespace tut {
//
// Test Group
//

using geos::geom::util::GeometryFixer;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Point;

// Common data used by tests
struct test_geometryfixer_data {

    geos::io::WKTReader wktreader_;
    geos::io::WKTWriter wktwriter_;

    test_geometryfixer_data()
    {
    }

    void checkFix(const std::string& wkt, const std::string& wktExpected) {
        std::unique_ptr<Geometry> geom = wktreader_.read(wkt);
        checkFix(geom.get(), false, wktExpected);
    }

    void checkFixKeepCollapse(const std::string& wkt, const std::string& wktExpected) {
        std::unique_ptr<Geometry> geom = wktreader_.read(wkt);
        checkFix(geom.get(), true, wktExpected);
    }

    void checkFix(const Geometry* input, const std::string& wktExpected) {
        checkFix(input, false, wktExpected);
    }

    void checkFixKeepCollapse(const Geometry* input, const std::string& wktExpected) {
        checkFix(input, true, wktExpected);
    }

    void checkFix(const Geometry* input, bool keepCollapse, const std::string& wktExpected) {

        std::unique_ptr<Geometry> actual;
        if (keepCollapse) {
            GeometryFixer fixer(input);
            fixer.setKeepCollapsed(true);
            actual = fixer.getResult();
        }
        else {
            actual = GeometryFixer::fix(input);
        }

        std::unique_ptr<Geometry> expected = wktreader_.read(wktExpected);

        // std::cout << "Reslt: " << wktwriter_.write(actual.get()) << std::endl;
        // std::cout << "Expct: " << wktwriter_.write(expected.get()) << std::endl;

        ensure("Result is invalid", actual->isValid());
        ensure_equals_geometry(expected.get(), actual.get());
    }


    void checkFixZ(const std::string& wkt, const std::string& wktExpected) {
        std::unique_ptr<Geometry> geom = wktreader_.read(wkt);
        checkFixZ(geom.get(), false, wktExpected);
    }

    void checkFixZKeepCollapse(const std::string& wkt, const std::string& wktExpected) {
        std::unique_ptr<Geometry> geom = wktreader_.read(wkt);
        checkFixZ(geom.get(), true, wktExpected);
    }

    void checkFixZ(const Geometry* input, bool keepCollapse, const std::string& wktExpected) {
        std::unique_ptr<Geometry> actual;
        if (keepCollapse) {
            GeometryFixer fixer(input);
            fixer.setKeepCollapsed(true);
            actual = fixer.getResult();
        }
        else {
            actual = GeometryFixer::fix(input);
        }

        std::unique_ptr<Geometry> expected = wktreader_.read(wktExpected);

        actual->normalize();
        expected->normalize();

        ensure("Result is invalid", actual->isValid());
        ensure_equals_geometry(expected.get(), actual.get());

        std::string actualWKT = wktwriter_.write(actual.get());
        std::string expectedWKT = wktwriter_.write(expected.get());
        ensure_equals(actualWKT, expectedWKT);
    }

    std::unique_ptr<Point> createPoint(double x, double y) {
        geos::geom::Coordinate p(x, y);
        GeometryFactory::Ptr fact = GeometryFactory::create();
        std::unique_ptr<Point> pt(fact->createPoint(p));
        return pt;
    }
};

typedef test_group<test_geometryfixer_data, 255> group;
typedef group::object object;

group test_geometryfixer_group("geos::geom::util::GeometryFixer");

template<>
template<>
void object::test<1>()
{
    checkFix("POINT (0 0)", "POINT (0 0)");
}

// testPointNaN
template<>
template<>
void object::test<2>()
{
    checkFix("POINT (0 Nan)", "POINT EMPTY");
}

// testPointEmpty
template<>
template<>
void object::test<3>()
{
    checkFix("POINT EMPTY", "POINT EMPTY");
}

// testPointPosInf
template<>
template<>
void object::test<4>()
{
    std::unique_ptr<Point> pt = createPoint(0, geos::DoubleInfinity);
    checkFix(pt.get() , "POINT EMPTY");
}

// testPointNegInf
template<>
template<>
void object::test<5>()
{
    std::unique_ptr<Point> pt = createPoint(0, geos::DoubleInfinity);
    checkFix(pt.get() , "POINT EMPTY");
}

//----------------------------------------

// testMultiPointNaN
template<>
template<>
void object::test<6>()
{
    checkFix("MULTIPOINT ((0 Nan))",
        "MULTIPOINT EMPTY");
}

// testMultiPoint
template<>
template<>
void object::test<7>()
{
    checkFix("MULTIPOINT ((0 0), (1 1))",
        "MULTIPOINT ((0 0), (1 1))");
}

// testMultiPointWithEmpty
template<>
template<>
void object::test<8>()
{
    checkFix("MULTIPOINT ((0 0), EMPTY)",
        "MULTIPOINT ((0 0))");
}

// testMultiPointWithMultiEmpty
template<>
template<>
void object::test<9>()
{
    checkFix("MULTIPOINT (EMPTY, EMPTY)",
        "MULTIPOINT EMPTY");
}

//----------------------------------------

// testLineStringEmpty
template<>
template<>
void object::test<10>()
{
    checkFix("LINESTRING EMPTY",
        "LINESTRING EMPTY");
}

// testLineStringCollapse
template<>
template<>
void object::test<11>()
{
    checkFix("LINESTRING (0 0, 1 NaN, 0 0)",
        "LINESTRING EMPTY");
}

// testLineStringCollapseMultipleRepeated
template<>
template<>
void object::test<12>()
{
    checkFix("LINESTRING (0 0, 0 0, 0 0)",
        "LINESTRING EMPTY");
}

// testLineStringKeepCollapse
template<>
template<>
void object::test<13>()
{
    checkFixKeepCollapse("LINESTRING (0 0, 0 0, 0 0)",
        "POINT (0 0)");
}

// testLineStringRepeated
template<>
template<>
void object::test<14>()
{
    checkFix("LINESTRING (0 0, 0 0, 0 0, 0 0, 0 0, 1 1)",
        "LINESTRING (0 0, 1 1)");
}

  /**
   * Checks that self-crossing are valid, and that entire geometry is copied
   */
// testLineStringSelfCross
template<>
template<>
void object::test<15>()
{
    checkFix("LINESTRING (0 0, 9 9, 9 5, 0 5)",
        "LINESTRING (0 0, 9 9, 9 5, 0 5)");
}

  //----------------------------------------

// testLinearRingEmpty
template<>
template<>
void object::test<16>()
{
    checkFix("LINEARRING EMPTY",
        "LINEARRING EMPTY");
}

// testLinearRingCollapsePoint
template<>
template<>
void object::test<17>()
{
    checkFix("LINEARRING (0 0, 1 NaN, 0 0)",
        "LINEARRING EMPTY");
}

// testLinearRingCollapseLine
template<>
template<>
void object::test<18>()
{
    checkFix("LINEARRING (0 0, 1 NaN, 1 0, 0 0)",
        "LINEARRING EMPTY");
}

// testLinearRingKeepCollapsePoint
template<>
template<>
void object::test<19>()
{
    checkFixKeepCollapse(
        "LINEARRING (0 0, 1 NaN, 0 0)",
        "POINT (0 0)");
}

// testLinearRingKeepCollapseLine
template<>
template<>
void object::test<20>()
{
    checkFixKeepCollapse(
        "LINEARRING (0 0, 1 NaN, 1 0, 0 0)",
        "LINESTRING (0 0, 1 0, 0 0)");
}

// testLinearRingValid
template<>
template<>
void object::test<21>()
{
    checkFix("LINEARRING (10 10, 10 90, 90 90, 90 10, 10 10)",
             "LINEARRING (10 10, 10 90, 90 90, 90 10, 10 10)");
}

// testLinearRingFlat
template<>
template<>
void object::test<22>()
{
    checkFix("LINEARRING (10 10, 10 90, 90 90, 10 90, 10 10)",
             "LINESTRING (10 10, 10 90, 90 90, 10 90, 10 10)");
}

/**
* Checks that invalid self-crossing ring is returned as a LineString
*/
// testLinearRingSelfCross
template<>
template<>
void object::test<23>()
{
    checkFix("LINEARRING (10 10, 10 90, 90 10, 90 90, 10 10)",
             "LINESTRING (10 10, 10 90, 90 10, 90 90, 10 10)");
}

  //----------------------------------------

// testMultiLineStringSelfCross
template<>
template<>
void object::test<24>()
{
    checkFix("MULTILINESTRING ((10 90, 90 10, 90 90), (90 50, 10 50))",
             "MULTILINESTRING ((10 90, 90 10, 90 90), (90 50, 10 50))");
}

// testMultiLineStringWithCollapse
template<>
template<>
void object::test<25>()
{
    checkFix(
        "MULTILINESTRING ((10 10, 90 90), (10 10, 10 10, 10 10))",
        "LINESTRING (10 10, 90 90)");
}

// testMultiLineStringKeepCollapse
template<>
template<>
void object::test<26>()
{
    checkFixKeepCollapse(
        "MULTILINESTRING ((10 10, 90 90), (10 10, 10 10, 10 10))",
        "GEOMETRYCOLLECTION (POINT (10 10), LINESTRING (10 10, 90 90))");
}

// testMultiLineStringWithEmpty
template<>
template<>
void object::test<27>()
{
    checkFix(
        "MULTILINESTRING ((10 10, 90 90), EMPTY)",
        "LINESTRING (10 10, 90 90)");
}

// testMultiLineStringWithMultiEmpty
template<>
template<>
void object::test<28>()
{
    checkFix(
        "MULTILINESTRING (EMPTY, EMPTY)",
        "MULTILINESTRING EMPTY");
}

  //----------------------------------------

// testPolygonEmpty
template<>
template<>
void object::test<29>()
{
    checkFix("POLYGON EMPTY",
        "POLYGON EMPTY");
}

// testPolygonBowtie
template<>
template<>
void object::test<30>()
{
    checkFix("POLYGON ((10 90, 90 10, 90 90, 10 10, 10 90))",
        "MULTIPOLYGON (((10 90, 50 50, 10 10, 10 90)), ((50 50, 90 90, 90 10, 50 50)))");
}

// testPolygonHolesZeroAreaOverlapping
template<>
template<>
void object::test<31>()
{
    checkFix(
        "POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (80 70, 30 70, 30 20, 30 70, 80 70), (70 80, 70 30, 20 30, 70 30, 70 80))",
        "POLYGON ((90 90, 90 10, 10 10, 10 90, 90 90))");
}

// testPolygonPosAndNegOverlap
template<>
template<>
void object::test<32>()
{
    checkFix(
        "POLYGON ((10 90, 50 90, 50 30, 70 30, 70 50, 30 50, 30 70, 90 70, 90 10, 10 10, 10 90))",
        "POLYGON ((10 90, 50 90, 50 70, 90 70, 90 10, 10 10, 10 90), (50 50, 50 30, 70 30, 70 50, 50 50))");
}

// testHolesTouching
template<>
template<>
void object::test<33>()
{
    checkFix(
        "POLYGON ((0 0, 0 5, 6 5, 6 0, 0 0), (3 1, 4 1, 4 2, 3 2, 3 1), (3 2, 1 4, 5 4, 4 2, 4 3, 3 2, 2 3, 3 2))",
        "MULTIPOLYGON (((0 0, 0 5, 6 5, 6 0, 0 0), (1 4, 2 3, 3 2, 3 1, 4 1, 4 2, 5 4, 1 4)), ((3 2, 4 3, 4 2, 3 2)))");
}

// testPolygonNaN
template<>
template<>
void object::test<34>()
{
    checkFix(
        "POLYGON ((10 90, 90 NaN, 90 10, 10 10, 10 90))",
        "POLYGON ((10 10, 10 90, 90 10, 10 10))");
}

// testPolygonRepeated
template<>
template<>
void object::test<35>()
{
    checkFix(
        "POLYGON ((10 90, 90 10, 90 10, 90 10, 90 10, 90 10, 10 10, 10 90))",
        "POLYGON ((10 10, 10 90, 90 10, 10 10))");
}

// testPolygonShellCollapse
template<>
template<>
void object::test<36>()
{
    checkFix("POLYGON ((10 10, 10 90, 90 90, 10 90, 10 10), (20 80, 60 80, 60 40, 20 40, 20 80))",
        "POLYGON EMPTY");
}

// testPolygonShellCollapseNaN
template<>
template<>
void object::test<37>()
{
    checkFix("POLYGON ((10 10, 10 NaN, 90 NaN, 10 NaN, 10 10))",
        "POLYGON EMPTY");
}

// testPolygonShellKeepCollapseNaN
template<>
template<>
void object::test<38>()
{
    checkFixKeepCollapse("POLYGON ((10 10, 10 NaN, 90 NaN, 10 NaN, 10 10))",
        "POINT (10 10)");
}

// testPolygonShellKeepCollapse
template<>
template<>
void object::test<39>()
{
    checkFixKeepCollapse("POLYGON ((10 10, 10 90, 90 90, 10 90, 10 10), (20 80, 60 80, 60 40, 20 40, 20 80))",
        "LINESTRING (10 10, 10 90, 90 90, 10 90, 10 10)");
}

// testPolygonHoleCollapse
template<>
template<>
void object::test<40>()
{
    checkFix("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (80 80, 20 80, 20 20, 20 80, 80 80))",
        "POLYGON ((10 10, 10 90, 90 90, 90 10, 10 10))");
}

// testPolygonHoleKeepCollapse
template<>
template<>
void object::test<41>()
{
    checkFixKeepCollapse("POLYGON ((10 90, 90 90, 90 10, 10 10, 10 90), (80 80, 20 80, 20 20, 20 80, 80 80))",
        "POLYGON ((10 10, 10 90, 90 90, 90 10, 10 10))");
}

  //----------------------------------------

// testMultiPolygonEmpty
template<>
template<>
void object::test<42>()
{
    checkFix(
        "MULTIPOLYGON EMPTY",
        "MULTIPOLYGON EMPTY");
}

// testMultiPolygonMultiEmpty
template<>
template<>
void object::test<43>()
{
    checkFix(
        "MULTIPOLYGON (EMPTY, EMPTY)",
        "MULTIPOLYGON EMPTY");
}

// testMultiPolygonWithEmpty
template<>
template<>
void object::test<44>()
{
    checkFix(
        "MULTIPOLYGON (((10 40, 40 40, 40 10, 10 10, 10 40)), EMPTY, ((50 40, 80 40, 80 10, 50 10, 50 40)))",
        "MULTIPOLYGON (((10 40, 40 40, 40 10, 10 10, 10 40)), ((50 40, 80 40, 80 10, 50 10, 50 40)))");
}

// testMultiPolygonWithCollapse
template<>
template<>
void object::test<45>()
{
    checkFix(
        "MULTIPOLYGON (((10 40, 40 40, 40 10, 10 10, 10 40)), ((50 40, 50 40, 50 40, 50 40, 50 40)))",
        "POLYGON ((10 10, 10 40, 40 40, 40 10, 10 10))");
}

// testMultiPolygonKeepCollapse
template<>
template<>
void object::test<46>()
{
    checkFixKeepCollapse(
        "MULTIPOLYGON (((10 40, 40 40, 40 10, 10 10, 10 40)), ((50 40, 50 40, 50 40, 50 40, 50 40)))",
        "GEOMETRYCOLLECTION (POINT (50 40), POLYGON ((10 10, 10 40, 40 40, 40 10, 10 10)))");
}

  //----------------------------------------

// testGCEmpty
template<>
template<>
void object::test<47>()
{
    checkFix(
        "GEOMETRYCOLLECTION EMPTY",
        "GEOMETRYCOLLECTION EMPTY");
}


// testGCWithAllEmpty
template<>
template<>
void object::test<48>()
{
    checkFix(
        "GEOMETRYCOLLECTION (POINT EMPTY, LINESTRING EMPTY, POLYGON EMPTY)",
        "GEOMETRYCOLLECTION (POINT EMPTY, LINESTRING EMPTY, POLYGON EMPTY)");
}

template<>
template<>
void object::test<49>()
{
    checkFixZ(
        "POLYGON Z ((10 90 1, 90 10 9, 90 90 9, 10 10 1, 10 90 1))",
        "MULTIPOLYGON Z(((10 10 1, 10 90 1, 50 50 5, 10 10 1)), ((50 50 5, 90 90 9, 90 10 9, 50 50 5)))");
}

// testPolygonHoleOverlapAndOutsideOverlap
template<>
template<>
void object::test<50>()
{
    checkFix(
        "POLYGON ((50 90, 80 90, 80 10, 50 10, 50 90), (70 80, 90 80, 90 20, 70 20, 70 80), (40 80, 40 50, 0 50, 0 80, 40 80), (30 40, 10 40, 10 60, 30 60, 30 40), (60 70, 80 70, 80 30, 60 30, 60 70))",
        "MULTIPOLYGON (((10 40, 10 50, 0 50, 0 80, 40 80, 40 50, 30 50, 30 40, 10 40)), ((70 80, 70 70, 60 70, 60 30, 70 30, 70 20, 80 20, 80 10, 50 10, 50 90, 80 90, 80 80, 70 80)))");
}

template<>
template<>
void object::test<51>()
{
    checkFixZ(
        "POLYGON Z ((10 90 1, 60 90 6, 60 10 6, 10 10 1, 10 90 1), (20 80 2, 90 80 9, 90 20 9, 20 20 2, 20 80 2))",
        "POLYGON Z((10 10 1, 10 90 1, 60 90 6, 60 80 6, 20 80 2, 20 20 2, 60 20 6, 60 10 6, 10 10 1))");
}

template<>
template<>
void object::test<52>()
{
    checkFixZKeepCollapse(
        "MULTILINESTRING Z ((10 10 1, 90 90 9), (10 10 1, 10 10 2, 10 10 3))",
        "GEOMETRYCOLLECTION Z (POINT (10 10 1), LINESTRING (10 10 1, 90 90 9))");
}



} // namespace tut
