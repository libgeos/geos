//
// Test Suite for geos::operation::overlayng::OverlayNGRobust class.
//
// Useful place for test cases raised by 3rd party software, that will be using
// Geometry->Intersection(), Geometry->Union(), etc, that call into the
// OverlayNGRobust utility class that bundles up different precision models
// and noders to provide a "best case" overlay for all inputs.
//

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNGRobust.h>

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
struct test_overlayngrobust_data {

    WKTReader r;
    WKTWriter w;

    void
    testOverlay(const std::string& a, const std::string& b, const std::string& expected, int opCode)
    {
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = OverlayNGRobust::Overlay(geom_a.get(), geom_b.get(), opCode);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get());
    }

};

typedef test_group<test_overlayngrobust_data> group;
typedef group::object object;

group test_overlayngrobust_data("geos::operation::overlayng::OverlayNGRobust");

//
// Test Cases
//

// 2020-09-17, GDAL exposed error in result clipping routine
template<>
template<>
void object::test<1> ()
{
    std::string a = "LINESTRING(832864.275023695 0,835092.849076364 0)";
    std::string b = "MULTIPOLYGON (((832864.275023695 0.0,833978.556808034 -0.000110682755987,833978.556808034 0.0,833978.556808034 0.000110682755987,832864.275023695 0.0,832864.275023695 0.0)),((835092.849076364 0.0,833978.557030887 -0.000110682755987,833978.557030887 0.0,833978.557030887 0.000110682755987,835092.849076364 0.0,835092.849076364 0.0)))";
    std::string exp = "MULTILINESTRING ((832864.275023695 0.0,833978.556808034 0.0),(833978.557030887 0.0,835092.849076364 0.0))";
    testOverlay(a, b, exp, OverlayNG::INTERSECTION);
}



} // namespace tut
