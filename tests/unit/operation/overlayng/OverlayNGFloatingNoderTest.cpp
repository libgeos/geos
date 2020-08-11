//
// Test Suite for geos::operation::overlayng::OverlayNG class with SnappingNoder.

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
struct test_overlayngfloating_data {

    WKTReader r;
    WKTWriter w;

    std::unique_ptr<Geometry>
    geomOverlay(const std::string& a, const std::string& b, int opCode)
    {
        std::unique_ptr<Geometry> geom_a = r.read(a);
        std::unique_ptr<Geometry> geom_b = r.read(b);
        return OverlayNG::overlay(geom_a.get(), geom_b.get(), opCode);
    }

    void
    geomIntersection(const std::string& a, const std::string& b, const std::string& expected, double checkTolerance)
    {
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = geomOverlay(a, b, OverlayNG::INTERSECTION);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get(), checkTolerance);
    }

    void
    geomUnion(const std::string& a, const std::string& b, const std::string& expected, double checkTolerance)
    {
        std::unique_ptr<Geometry> geom_expected = r.read(expected);
        std::unique_ptr<Geometry> geom_result = geomOverlay(a, b, OverlayNG::UNION);
        // std::string wkt_result = w.write(geom_result.get());
        // std::cout << std::endl << wkt_result << std::endl;
        ensure_equals_geometry(geom_expected.get(), geom_result.get(), checkTolerance);
    }

    double
    intersectionAreaExpectError(const std::string& a, const std::string& b)
    {
        try {
            std::unique_ptr<Geometry> geom_result =
                geomOverlay(a, b, OverlayNG::INTERSECTION);
            return geom_result->getArea();
        }
        catch (geos::util::TopologyException &) {
            // This exception is expected if the
            // geometries are not perturbed by clipping
        }
        return 0.0;
    }

};

typedef test_group<test_overlayngfloating_data> group;
typedef group::object object;

group test_overlayngfloating_group("geos::operation::overlayng::OverlayNGFloatingNoder");

//
// Test Cases
//


// testTriangleIntersection
template<>
template<>
void object::test<1> ()
{
    std::string a = "POLYGON ((0 0, 8 0, 8 3, 0 0))";
    std::string b = "POLYGON ((0 5, 5 0, 0 0, 0 5))";
    std::string expected = "POLYGON ((0 0, 3.6363636363636367 1.3636363636363638, 5 0, 0 0))";
    geomIntersection(a, b, expected, 1e-10);
}


// testPolygonWithRepeatedPointIntersectionSimple
template<>
template<>
void object::test<2> ()
{
    std::string a = "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 151, 100 151, 100 151, 100 151, 100 200))";
    std::string b = "POLYGON ((300 200, 300 100, 200 100, 200 200, 200 200, 300 200))";
    std::string expected = "LINESTRING (200 200, 200 100)";
    geomIntersection(a, b, expected, 1e-10);
}

// testLineWithRepeatedPointIntersection
template<>
template<>
void object::test<3> ()
{
    std::string a = "LINESTRING (100 100, 200 200, 200 200, 200 200, 200 200, 300 300, 400 200)";
    std::string b = "LINESTRING (190 110, 120 180)";
    std::string expected = "POINT (150 150)";
    geomIntersection(a, b, expected, 1e-10);
}

// testPolygonWithRepeatedPointIntersection
template<>
template<>
void object::test<4> ()
{
    std::string a = "POLYGON ((1231646.6575 1042601.8724999996, 1231646.6575 1042601.8724999996, 1231646.6575 1042601.8724999996, 1231646.6575 1042601.8724999996, 1231646.6575 1042601.8724999996, 1231646.6575 1042601.8724999996, 1231646.6575 1042601.8724999996, 1231646.6575 1042601.8724999996, 1231647.72 1042600.4349999996, 1231653.22 1042592.1849999996, 1231665.14087406 1042572.5988970799, 1231595.8411746 1042545.58898314, 1231595.26811297 1042580.9672385901, 1231595.2825 1042582.8724999996, 1231646.6575 1042601.8724999996))";
    std::string b = "POLYGON ((1231665.14087406 1042572.5988970799, 1231665.14087406 1042572.5988970799, 1231665.14087406 1042572.5988970799, 1231665.14087406 1042572.5988970799, 1231665.14087406 1042572.5988970799, 1231665.14087406 1042572.5988970799, 1231665.14087406 1042572.5988970799, 1231665.14087406 1042572.5988970799, 1231666.51617512 1042570.3392651202, 1231677.47 1042558.9349999996, 1231685.50958834 1042553.8506523697, 1231603.31532446 1042524.6022436405, 1231603.31532446 1042524.6022436405, 1231603.31532446 1042524.6022436405, 1231603.31532446 1042524.6022436405, 1231596.4075 1042522.1849999996, 1231585.07346906 1042541.8167165304, 1231586.62051091 1042542.3586940402, 1231586.62051091 1042542.3586940402, 1231595.8411746 1042545.58898314, 1231665.14087406 1042572.5988970799))";
    std::unique_ptr<Geometry> actual = geomOverlay(a, b, OverlayNG::INTERSECTION);
    // test is ok if intersection computes without error
    bool isCorrect = actual->getArea() < 1;
    ensure("Area of intersection result area is too large", isCorrect);
}


/**
* Tests a case where ring clipping causes an incorrect result.
*
* The incorrect result occurs because:
*
*  - Ring Clipping causes a clipped A line segment to move slightly.
*  - This causes the clipped A and B edges to become disjoint
*    (whereas in the original geometry they intersected).
*  - Both edge rings are thus determined to be disconnected during overlay labeling.
*  - For the overlay labeling for the disconnected edge in geometry B,
*    the chosen edge coordinate has its location computed as inside the original A polygon.
*    This is because the chosen coordinate happens to be the one that the
*    clipped edge crossed over.
*  - This causes the (clipped) B edge ring to be labelled as Interior to the A polygon.
*  - The B edge ring thus is computed as being in the intersection,
*    and the entire ring is output, producing a much larger polygon than is correct.
*
* The test check here is a heuristic that detects the presence of a large
* polygon in the output.
*
* There are several possible fixes:
*
*  - Improve clipping to avoid clipping line segments which may intersect
*    other geometry (by computing a large enough clipping envelope)
*  - Improve choosing a point for disconnected edge location;
*    i.e. by finding one that is far from the other geometry edges.
*    However, this still creates a result which may not reflect the
*    actual input topology.
*
* The chosen fix is the first above - improve clipping
* by choosing a larger clipping envelope.
*
* NOTE: When clipping is improved to avoid perturbing intersecting segments,
* the floating overlay now reports a TopologyException.
* This is reported as an empty geometry to allow tests to pass.
*/

// xtestPolygonsWithClippingPerturbationIntersection
template<>
template<>
void object::test<5> ()
{
    std::string a = "POLYGON ((4373089.33 5521847.89, 4373092.24 5521851.6, 4373118.52 5521880.22, 4373137.58 5521896.63, 4373153.33 5521906.43, 4373270.51 5521735.67, 4373202.5 5521678.73, 4373100.1 5521827.97, 4373089.33 5521847.89))";
    std::string b = "POLYGON ((4373225.587574724 5521801.132991467, 4373209.219497436 5521824.985294571, 4373355.5585138 5521943.53124194, 4373412.83157427 5521860.49206234, 4373412.577392304 5521858.140878815, 4373412.290476093 5521855.48690386, 4373374.245799139 5521822.532711867, 4373271.028377312 5521736.104060946, 4373225.587574724 5521801.132991467))";
    double area = intersectionAreaExpectError(a, b);
    bool isCorrect = area < 1;
    ensure("Area of intersection result area is too large", isCorrect);
}

// xtestPolygonsWithClippingPerturbation2Intersection
template<>
template<>
void object::test<6> ()
{
    std::string a = "POLYGON ((4379891.12 5470577.74, 4379875.16 5470581.54, 4379841.77 5470592.88, 4379787.53 5470612.89, 4379822.96 5470762.6, 4379873.52 5470976.3, 4379982.93 5470965.71, 4379936.91 5470771.25, 4379891.12 5470577.74))";
    std::string b = "POLYGON ((4379894.528437099 5470592.144163859, 4379968.579210246 5470576.004727546, 4379965.600743549 5470563.403176092, 4379965.350009631 5470562.383524827, 4379917.641365346 5470571.523966022, 4379891.224959933 5470578.183564024, 4379894.528437099 5470592.144163859))";
    double area = intersectionAreaExpectError(a, b);
    bool isCorrect = area < 1;
    ensure("Area of intersection result area is too large", isCorrect);
}




} // namespace tut
