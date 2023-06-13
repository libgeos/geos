//
// Test Suite for geos::geom::HeuristicOverlay

#include <tut/tut.hpp>

// geos
#include <geos/algorithm/hull/ConcaveHull.h>
#include <geos/constants.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/HeuristicOverlay.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <utility.h>

// std
#include <string>
#include <memory>

namespace tut {
//
// Test Group
//

using geos::operation::overlayng::OverlayNG;

struct test_heuristic_data {

    WKTReader reader_;
    WKTWriter writer_;

    test_heuristic_data() {}

    void checkOverlay(
        const std::string& wkt1,
        const std::string& wkt2,
        int opCode,
        const std::string& wkt_expected)
    {
        std::unique_ptr<Geometry> g1 = reader_.read(wkt1);
        std::unique_ptr<Geometry> g2 = reader_.read(wkt2);
        std::unique_ptr<Geometry> expected = reader_.read(wkt_expected);
        std::unique_ptr<Geometry> actual = HeuristicOverlay(g1.get(), g2.get(), opCode);

        // std::cout << "expected:" << std::endl << writer_.write(*expected) << std::endl;
        // std::cout << "actual:" << std::endl << writer_.write(*actual) << std::endl;

        ensure_equals_geometry(expected.get(), actual.get());
    }


};

typedef test_group<test_heuristic_data> group;
typedef group::object object;

group test_heuristic_data("geos::geom::HeuristicOverlay");

//
// Test Cases
//

//
// These tests exercise the special cast code in HeuristicOverlay
// for GeometryCollection in which the contents are "mixed dimension",
// such as points and lines or lines and polygons in the same collection.
// For those cases the result of the overlay might be a matter of
// interpretation, depending on the inputs and the opinions of the
// end user. The implementation just tries to generate a visually
// defensible, simplified answer.
//

template<>
template<>
void object::test<1> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POINT(0 0), LINESTRING(1 1, 2 2))",
        "GEOMETRYCOLLECTION(POINT(10 10), LINESTRING(11 11, 12 12))",
        OverlayNG::UNION,
        "GEOMETRYCOLLECTION(POINT(0 0), LINESTRING(1 1, 2 2), POINT(10 10), LINESTRING(11 11, 12 12))"
        );
}

template<>
template<>
void object::test<2> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POINT(0 0), LINESTRING(1 1, 2 2))",
        "POLYGON((-10 -10, -10 10, 10 10, 10 -10, -10 -10))",
        OverlayNG::UNION,
        "POLYGON((-10 -10, -10 10, 10 10, 10 -10, -10 -10))"
        );
}

template<>
template<>
void object::test<3> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POINT(0.5 0.5), LINESTRING(0 0, 2 2), POLYGON((0 0, 1 0, 1 1, 0 1, 0 0)))",
        "GEOMETRYCOLLECTION(LINESTRING(0.5 0.5, 0.5 4), POINT(2 0))",
        OverlayNG::UNION,
        "GEOMETRYCOLLECTION (POINT (2 0), LINESTRING (0.5 1, 0.5 4), LINESTRING (1 1, 2 2), POLYGON ((0 1, 1 1, 1 0, 0 0, 0 1)))"
        );
}

template<>
template<>
void object::test<4> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(20 20, 30 30))",
        "GEOMETRYCOLLECTION(POLYGON((9 9, 21 9, 21 21, 9 21, 9 9)), POINT(5 5))",
        OverlayNG::DIFFERENCE,
        "GEOMETRYCOLLECTION (LINESTRING (21 21, 30 30), POLYGON ((10 0, 0 0, 0 10, 9 10, 9 9, 10 9, 10 0)))"
        );
}

template<>
template<>
void object::test<5> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(20 20, 30 30))",
        "GEOMETRYCOLLECTION(POLYGON((9 9, 21 9, 21 21, 9 21, 9 9)), POINT(5 5))",
        OverlayNG::INTERSECTION,
        "GEOMETRYCOLLECTION (POINT (5 5), LINESTRING(20 20, 21 21), POLYGON ((10 10, 10 9, 9 9, 9 10, 10 10)))"
        );
}

template<>
template<>
void object::test<6> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)), LINESTRING(20 20, 30 30))",
        "GEOMETRYCOLLECTION(POLYGON((9 9, 21 9, 21 21, 9 21, 9 9)), POINT(5 5))",
        OverlayNG::SYMDIFFERENCE,
        "GEOMETRYCOLLECTION (LINESTRING (21 21, 30 30), POLYGON ((0 0, 0 10, 9 10, 9 9, 10 9, 10 0, 0 0)), POLYGON ((9 10, 9 21, 21 21, 21 9, 10 9, 10 10, 9 10)))"
        );
}


template<>
template<>
void object::test<7> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)))",
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)))",
        OverlayNG::UNION,
        "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))"
        );
}

template<>
template<>
void object::test<8> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)))",
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)), POINT(20 20))",
        OverlayNG::DIFFERENCE,
        "GEOMETRYCOLLECTION EMPTY"
        );
}

template<>
template<>
void object::test<9> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)))",
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)))",
        OverlayNG::INTERSECTION,
        "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))"
        );
}


template<>
template<>
void object::test<10> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)), POINT EMPTY, MULTIPOINT(4 4, 11 11), LINESTRING(5 5, 6 6))",
        "GEOMETRYCOLLECTION(POLYGON((2 2, 12 2, 12 12, 2 12, 2 2)), LINESTRING EMPTY, MULTIPOINT(4 4, 11 11), LINESTRING(5 6, 6 5))",
        OverlayNG::INTERSECTION,
        "GEOMETRYCOLLECTION (POINT (11 11), POLYGON ((10 10, 10 2, 2 2, 2 10, 10 10)))"
        );
}

template<>
template<>
void object::test<11> ()
{
    checkOverlay(
        "GEOMETRYCOLLECTION(POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)), POINT EMPTY, MULTIPOINT(4 4, 11 11), LINESTRING(5 5, 6 6))",
        "GEOMETRYCOLLECTION(POLYGON((2 2, 12 2, 12 12, 2 12, 2 2)), LINESTRING EMPTY, MULTIPOINT(4 4, 11 11), LINESTRING(5 6, 6 5))",
        OverlayNG::UNION,
        "POLYGON ((2 12, 12 12, 12 2, 10 2, 10 0, 0 0, 0 10, 2 10, 2 12))"
        );
}

} // namespace tut
