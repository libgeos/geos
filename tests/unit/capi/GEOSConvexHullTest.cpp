//
// Test Suite for C-API GEOSConvexHull

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosconvexhull_data : public capitest::utility {
};

typedef test_group<test_capigeosconvexhull_data> group;
typedef group::object object;

group test_capigeosconvexhull_group("capi::GEOSConvexHull");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    input_ = GEOSGeomFromWKT("MULTIPOINT ((130 240), (130 240), (570 240), (570 290), (650 240))");
    ensure(nullptr != input_);

    expected_ = GEOSGeomFromWKT("POLYGON ((130 240, 570 290, 650 240, 130 240))");
    ensure(nullptr != expected_);

    GEOSGeometry* output = GEOSConvexHull(input_);
    ensure(nullptr != output);
    ensure(0 == GEOSisEmpty(output));
    ensure_geometry_equals(output, expected_);
    GEOSGeom_destroy(output);
}

} // namespace tut

