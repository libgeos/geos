//
// Test Suite for geos::operation::overlayng::OverlayPoints class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNG.h>

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
struct test_overlayngpoints_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode, double scaleFactor)
    {
        PrecisionModel pm(scaleFactor);
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode, &pm);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

};

typedef test_group<test_overlayngpoints_data> group;
typedef group::object object;

group test_overlayngpoints_group("geos::operation::overlayng::OverlayNGPoints");

//
// Test Cases
//

//  testSimpleIntersection
template<>
template<>
void object::test<1> ()
{
    std::string a = "MULTIPOINT ((1 1), (2 1))";
    std::string b = "POINT (2 1)";
    std::string exp = "POINT (2 1)";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}


// testSimpleMergeIntersection
template<>
template<>
void object::test<2> ()
{
    std::string a = "MULTIPOINT ((1 1), (1.5 1.1), (2 1), (2.1 1.1))";
    std::string b = "POINT (2 1)";
    std::string exp = "POINT (2 1)";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// testSimpleUnion
template<>
template<>
void object::test<3> ()
{
    std::string a = "MULTIPOINT ((1 1), (2 1))";
    std::string b = "POINT (2 1)";
    std::string exp = "MULTIPOINT ((1 1), (2 1))";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}

// testSimpleDifference
template<>
template<>
void object::test<4> ()
{
    std::string a = "MULTIPOINT ((1 1), (2 1))";
    std::string b = "POINT (2 1)";
    std::string exp = "POINT (1 1)";
    testOverlay(a, b, exp, OverlayNG::DIFFERENCE, 1);
}

// testSimpleSymDifference
template<>
template<>
void object::test<5> ()
{
    std::string a = "MULTIPOINT ((1 2), (1 1), (2 2), (2 1))";
    std::string b = "MULTIPOINT ((2 2), (2 1), (3 2), (3 1))";
    std::string exp = "MULTIPOINT ((1 2), (1 1), (3 2), (3 1))";
    testOverlay(a, b, exp, OverlayNG::SYMDIFFERENCE, 1);
}

// testSimpleFloatUnion
template<>
template<>
void object::test<6> ()
{
    std::string a = "MULTIPOINT ((1 1), (1.5 1.1), (2 1), (2.1 1.1))";
    std::string b = "MULTIPOINT ((1.5 1.1), (2 1), (2 1.2))";
    std::string exp = "MULTIPOINT ((1 1), (1.5 1.1), (2 1), (2 1.2), (2.1 1.1))";
    testOverlay(a, b, exp, OverlayNG::UNION, 1000);
}

// testDisjointPointsRoundedIntersection
template<>
template<>
void object::test<7> ()
{
    std::string a = "POINT (10.1 10)";
    std::string b = "POINT (10 10.1)";
    std::string exp = "POINT (10 10)";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// testEmptyIntersection
template<>
template<>
void object::test<8> ()
{
    std::string a = "MULTIPOINT ((1 1), (3 1))";
    std::string b = "POINT (2 1)";
    std::string exp = "POINT EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// testEmptyInputIntersection
template<>
template<>
void object::test<9> ()
{
    std::string a = "MULTIPOINT ((1 1), (3 1))";
    std::string b = "POINT EMPTY";
    std::string exp = "POINT EMPTY";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION, 1);
}

// testEmptyInputUUnion
template<>
template<>
void object::test<10> ()
{
    std::string a = "MULTIPOINT ((1 1), (3 1))";
    std::string b = "POINT EMPTY";
    std::string exp = "MULTIPOINT ((1 1), (3 1))";
    testOverlay(a, b, exp, OverlayNG::UNION, 1);
}

// testEmptyDifference
template<>
template<>
void object::test<11> ()
{
    std::string a = "MULTIPOINT ((1 1), (3 1))";
    std::string b = "MULTIPOINT ((1 1), (2 1), (3 1))";
    std::string exp = "POINT EMPTY";
    testOverlay(a, b, exp, OverlayNG::DIFFERENCE, 1);
}




} // namespace tut
