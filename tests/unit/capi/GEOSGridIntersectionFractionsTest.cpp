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

    input_ = fromWKT("CURVEPOLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5))");

    std::vector<float> result_vec;
    int result = GEOSGridIntersectionFractions(input_, 1, 0, 5, 3, 4, 3, result_vec.data());
    ensure_equals(result, 0);
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

}
