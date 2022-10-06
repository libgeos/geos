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

using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
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
    GeometryFactory::Ptr factory;

    test_overlayngrobust_data() : factory(GeometryFactory::create()) {};

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

    std::unique_ptr<Geometry>
    double2geom(const std::vector<double>& x, const std::vector<double>& y)
    {
        auto coords = geos::detail::make_unique<CoordinateSequence>();
        for (std::size_t i = 0; i < x.size(); i++)
            coords->add(CoordinateXY{x[i], y[i]});
        std::unique_ptr<Geometry> geom = factory->createPolygon(factory->createLinearRing(std::move(coords)));
        return geom;
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



#if 0
/**
 * https://github.com/locationtech/jts/pull/821
 * https://github.com/locationtech/jts/issues/820
 *
 * ENABLE WHEN FIXES ARE IN
 */
template<>
template<>
void object::test<3> ()
{
    std::vector<double> x3 = {-13.621824029083443, -16.14144162383529, -16.15907384118054, -13.639456293556348, -13.621824029083443};
    std::vector<double> y3 = {0.15008489786842003, 0.10149068267229658, 1.0157206673651493, 1.0643148816523527, 0.15008489786842003};

    std::vector<double> x5 = {-12.707594043193543, -13.621824029083443, -13.639456293556348, -12.725226307666448, -12.707594043193543};
    std::vector<double> y5 = {0.1677170531469111, 0.15008489786842005, 1.0643148816523527, 1.0819470369308437, 0.1677170531469111};

    std::unique_ptr<Geometry> p3 = double2geom(x3, y3);
    std::unique_ptr<Geometry> p5 = double2geom(x5, y5);

    std::cout << *p3 << std::endl;

    std::cout << *p5 << std::endl;

    std::unique_ptr<Geometry> pUnion = OverlayNGRobust::Overlay(p3.get(), p5.get(), OverlayNG::UNION);

    std::cout << *pUnion << std::endl;

    std::cout << "p3->getArea(): " << p3->getArea() << std::endl;
    std::cout << "p5->getArea(): " << p5->getArea() << std::endl;
    double areaSum = p3->getArea() + p5->getArea();
    std::cout << "areaSum: " << areaSum << std::endl;
    double areaUnion = pUnion->getArea();
    std::cout << "areaUnion: " << areaUnion << std::endl;

    double areaDelta = std::fabs(areaUnion - areaSum);
    std::cout << "areaDelta: " << areaDelta << std::endl;
    double deltaFrac = areaDelta / std::max(areaUnion, areaSum);
    std::cout << "deltaFrac: " << deltaFrac << std::endl;
    ensure(deltaFrac < 0.1);
}

#endif

} // namespace tut
