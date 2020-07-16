//
// Test Suite for geos::operation::overlayng::OverlayNG class with SnappingNoder.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/noding/snap/SnappingNoder.h>
#include <geos/noding/ValidatingNoder.h>

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
struct test_overlayngsnappingone_data {

    WKTReader r;
    WKTWriter w;

    void
    geomUnionTest(const std::string& a, const std::string& b, const std::string& expected, double tolerance)
    {
        geos::noding::snap::SnappingNoder snapNoder(tolerance);
        geos::noding::ValidatingNoder validNoder(snapNoder);
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNG::overlay(geom_a.get(), geom_b.get(), OverlayNG::UNION, &validNoder);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

};

typedef test_group<test_overlayngsnappingone_data> group;
typedef group::object object;

group test_overlayngsnappingone_group("geos::operation::overlayng::OverlayNGSnappingOne");

//
// Test Cases
//


// testRectanglesOneAjarUnion
template<>
template<>
void object::test<1>()
{
    std::string a = "POLYGON ((10 10, 10 5, 5 5, 5 10, 10 10))";
    std::string b = "POLYGON ((10 15, 15 15, 15 7, 10.01 7, 10 15))";
    std::string expected = "POLYGON ((5 5, 5 10, 10 10, 10 15, 15 15, 15 7, 10.01 7, 10 5, 5 5))";
    geomUnionTest(a, b, expected, 1.0);
}

// testRectanglesBothAjarUnion
template<>
template<>
void object::test<2>()
{
    std::string a = "POLYGON ((10.01 10, 10 5, 5 5, 5 10, 10.01 10))";
    std::string b = "POLYGON ((10 15, 15 15, 15 7, 10.01 7, 10 15))";
    std::string expected = "POLYGON ((5 5, 5 10, 10.01 10, 10 15, 15 15, 15 7, 10.01 7, 10 5, 5 5))";
    geomUnionTest(a, b, expected, 1.0);
}

// testRandomUnion
template<>
template<>
void object::test<3>()
{
    std::string a = "POLYGON ((85.55954154387994 100, 92.87214039753759 100, 94.7254728121147 100, 98.69765702432045 96.38825885127041, 85.55954154387994 100))";
    std::string b = "POLYGON ((80.20688423699171 99.99999999999999, 100.00000000000003 99.99999999999997, 100.00000000000003 88.87471526860915, 80.20688423699171 99.99999999999999))";
    std::string expected = "POLYGON ((80.20688423699171 99.99999999999999, 85.55954154387994 100, 92.87214039753759 100, 94.7254728121147 100, 100.00000000000003 99.99999999999997, 100.00000000000003 88.87471526860915, 80.20688423699171 99.99999999999999))";
    geomUnionTest(a, b, expected, 0.00000001);
}

// testTrianglesBSegmentsDisplacedSmallTolUnion
template<>
template<>
void object::test<4>()
{
    std::string a = "POLYGON ((100 200, 200 0, 300 200, 100 200))";
    std::string b = "POLYGON ((150 200.01, 200 200.01, 260 200.01, 200 100, 150 200.01))";
    std::string expected = "POLYGON ((150 200.01, 200 200.01, 260 200.01, 300 200, 200 0, 100 200, 150 200.01))";
    geomUnionTest(a, b, expected, 0.01);
}

// testTrianglesBSegmentsDisplacedUnion
template<>
template<>
void object::test<5>()
{
    std::string a = "POLYGON ((100 200, 200 0, 300 200, 100 200))";
    std::string b = "POLYGON ((150 200.01, 200 200.01, 260 200.01, 200 100, 150 200.01))";
    std::string expected = "POLYGON ((100 200, 150 200.01, 200 200.01, 260 200.01, 300 200, 200 0, 100 200))";
    geomUnionTest(a, b, expected, 0.1);
}



} // namespace tut
