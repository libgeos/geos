//
// Test Suite for handling of Z in geos::operation::overlayng::OverlayNG class.
//
//
// Ported from JTS
// modules/core/src/test/java/org/locationtech/jts/operation/overlayng/OverlayNGZTest.java
// c1a2e9a3cd723ebfb6c79e9daf5217f562f470e0
//

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNG.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlayngz_data {

    WKTReader r;

    void checkOverlay(int opCode, const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> a = r.read(wktA);
        std::unique_ptr<Geometry> b = r.read(wktB);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        std::unique_ptr<Geometry> result = OverlayNG::overlay(a.get(), b.get(), opCode);
        ensure_equals_geometry_xyz(result.get(), expected.get());
    }

    void checkIntersection(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::INTERSECTION, wktA, wktB, wktExpected);
    }

    void checkDifference(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::DIFFERENCE, wktA, wktB, wktExpected);
    }

    void checkUnion(const std::string& wktA, const std::string& wktB, const std::string& wktExpected)
    {
        checkOverlay(OverlayNG::UNION, wktA, wktB, wktExpected);
    }

};

typedef test_group<test_overlayngz_data> group;
typedef group::object object;

group test_overlayngz_group("geos::operation::overlayng::OverlayNGZ");

//
// Test Cases
//

// testLineIntersectionPointInterpolated
template<>
template<>
void object::test<1> ()
{
    checkIntersection("LINESTRING (0 0 0, 10 10 10)", "LINESTRING (10 0 0, 0 10 10)", "POINT(5 5 5)");
}

// testLineIntersectionPoint
template<>
template<>
void object::test<2> ()
{
    checkIntersection(
        "LINESTRING (0 0 0, 10 10 10)",
        "LINESTRING (10 0 0, 5 5 999, 0 10 10)",
        "POINT(5 5 999)"
    );
}

//  testLineLineXYDifferenceLineInterpolate
template<>
template<>
void object::test<3> ()
{
    checkDifference(
        "LINESTRING (0 0 0, 10 10 10)",
        "LINESTRING (5 5, 6 6)",
        "MULTILINESTRING ((0 0 0, 5 5 5), (6 6 6, 10 10 10))"
    );
}

// testLineLineUnionOverlay
template<>
template<>
void object::test<4> ()
{
    checkUnion(
        "LINESTRING (0 0 0, 10 10 10)",
        "LINESTRING (5 5 990, 15 15 999)",
        "MULTILINESTRING Z((0 0 0, 5 5 990), (5 5 990, 10 10 10), (10 10 10, 15 15 999))"
    );
}

// testLinePolygonIntersectionLine
template<>
template<>
void object::test<5> ()
{
	checkIntersection(
		"LINESTRING Z (0 0 0, 5 5 5)",
		"POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
		"LINESTRING Z (1 1 1, 5 5 5)"
	);
}

// testLinePolygonDifferenceLine
template<>
template<>
void object::test<6> ()
{
	checkDifference(
		"LINESTRING Z (0 5 0, 10 5 10)",
		"POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
		"MULTILINESTRING Z((0 5 0, 1 5 2), (9 5 8, 10 5 10))"
	);
}

// testLinePolygonXYDifferenceLine
template<>
template<>
void object::test<7> ()
{
	checkDifference(
		"LINESTRING Z (0 5 0, 10 5 10)",
		"POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
		"MULTILINESTRING Z((0 5 0, 1 5 1), (9 5 9, 10 5 10))"
	);
}

//// TODO: add Z population from model
//// testLineXYPolygonDifferenceLine
//template<>
//template<>
//void object::test<8> ()
//{
//	checkDifference(
//		"LINESTRING (0 5, 10 5)",
//		"POLYGON Z ((1 9 5, 9 9 9, 9 1 5, 1 1 1, 1 9 5))",
//		"MULTILINESTRING Z((0 5 0, 1 5 2), (9 5 8, 10 5 10))"
//	);
//}




} // namespace tut
