//
// Test Suite for geos::operation::overlayng::OverlayNGRobust class.
//
// Useful place for test cases raised by 3rd party software, that will be using
// Geometry->Intersection(), Geometry->Union(), etc, that call into the
// OverlayNGRobust utility class that bundles up different precision models
// and noders to provide a "best case" overlay for all inputs.
//

#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/operation/overlayng/OverlayNG.h>

// std
#include <memory>

using namespace geos::geom;
using geos::io::WKTReader;
using geos::io::WKTWriter;
using geos::operation::overlayng::OverlayNGRobust;
using geos::operation::overlayng::OverlayNG;

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

    void
    checkOverlaySuccess(const std::string& a, const std::string& b, int opCode)
    {
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        ensure_NO_THROW( OverlayNGRobust::Overlay(geom_a.get(), geom_b.get(), opCode) );
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

/**
 * Tests correct ordering of {@link SegmentNode#compareTo(Object)}.
 *
 * See https://trac.osgeo.org/geos/ticket/1051
 *
 * JTS equivalent: testSegmentNodeOrderingIntersection()
 */
template<>
template<>
void object::test<2> ()
{
    std::string a = "POLYGON ((654948.3853299792 1794977.105854025, 655016.3812220972 1794939.918901604, 655016.2022581929 1794940.1099794197, 655014.9264068712 1794941.4254068714, 655014.7408834674 1794941.6101225375, 654948.3853299792 1794977.105854025))";
	std::string b = "POLYGON ((655103.6628454948 1794805.456674405, 655016.20226 1794940.10998, 655014.8317182435 1794941.5196832407, 655014.8295602322 1794941.5218318563, 655014.740883467 1794941.610122538, 655016.6029214273 1794938.7590508445, 655103.6628454948 1794805.456674405))";
    checkOverlaySuccess(a, b, OverlayNG::INTERSECTION);
}



} // namespace tut
