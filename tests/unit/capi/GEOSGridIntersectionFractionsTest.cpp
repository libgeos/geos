#include <tut/tut.hpp>
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosgridintersectionfractions_data : public capitest::utility {};

typedef test_group<test_capigeosgridintersectionfractions_data> group;
typedef group::object object;

group test_capigeosgridintersectionfractions_group("capi::GEOSGridIntersectionFractions");

template<>
template<>
void object::test<1>()
{
    set_test_name("basic rectangle");

    input_ = fromWKT("POLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5))");

    std::vector<float> result_vec(3*4);
    int result = GEOSGridIntersectionFractions(input_, 1, 0, 5, 3, 4, 3, result_vec.data());
    ensure_equals(result, 1);

    std::vector<float> expected = { 0.5, 0.25, 0, 0, 1, 0.5, 0, 0, 0.5, 0.25, 0, 0};

    ensure(result_vec == expected);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("curved input");
    useContext();

    input_ = fromWKT("CURVEPOLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5))");

    std::vector<float> result_vec(12);
    int result = GEOSGridIntersectionFractions_r(ctxt_, input_, 1, 0, 5, 3, 4, 3, result_vec.data());
    ensure_equals(result, 0);

    useCurveConversion();
    result = GEOSGridIntersectionFractions_r(ctxt_, input_, 1, 0, 5, 3, 4, 3, result_vec.data());
    ensure_equals(result, 1);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("non-areal input");

    input_ = fromWKT("POINT (3 8)");

    std::vector<float> result_vec;
    int result = GEOSGridIntersectionFractions(input_, 1, 0, 5, 3, 4, 3, result_vec.data());
    ensure_equals(result, 0);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("values outside the geometry bounding box are set to zero");

    input_ = fromWKT("POLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5))");

    const unsigned nx = 5;
    const unsigned ny = 5;
    const float init_value = 99999;

    std::vector<float> result_vec(nx * ny, init_value);
    int result = GEOSGridIntersectionFractions(input_, 0, 0, nx, ny, nx, ny, result_vec.data());
    ensure_equals(result, 1);

    std::vector<float> expected = {
        0,      0,    0, 0, 0,
        0,      0,    0, 0, 0,
        0.25, 0.5, 0.25, 0, 0,
        0.5,  1.0,  0.5, 0, 0,
        0.25, 0.5, 0.25, 0, 0
    };

    ensure(result_vec == expected);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("vertex at a corner of the geometry envelope and on a cell boundary");

    input_ = fromWKT("POLYGON ((-70.69363468506793 685.8491525449554, -133.48032295866494 748.6358408185523, -175.33811514106293 790.4936330009504, -259.05369950585896 811.4225290921494, -238.12480341465994 706.7780486361544, -259.05369950585896 664.9202564537563, -259.05369950585896 581.2046720889604, -196.26701123226195 643.9913603625573, -112.55142686746595 623.0624642713583, -70.69363468506793 685.8491525449554))");

    const double xmin = -300.91149168825694;
    const double ymin = 225.41343853857734;
    const double xmax = -175.33811514106293;
    const double ymax = 602.13356818015927;
    const unsigned nx = 6;
    const unsigned ny = 18;

    std::vector<float> result_vec(nx * ny);
    int result = GEOSGridIntersectionFractions(input_, xmin, ymin, xmax, ymax, nx, ny, result_vec.data());
    ensure_equals(result, 1);

    geom1_ = GEOSGeom_createRectangle(xmin, ymin, xmax, ymax);
    geom2_ = GEOSIntersection(input_, geom1_);
    double expected_area;
    GEOSArea(geom2_, &expected_area);

    double cell_area = (xmax - xmin) / nx * (ymax - ymin) / ny;
    double area = 0;
    for (float f : result_vec) {
        area += static_cast<double>(f) * cell_area;
    }

    ensure_equals("area", area, expected_area, 1e-6 * expected_area);
}

}
