// Test Suite for C-API BuildArea

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capi_buildarea_data : public capitest::utility {
    test_capi_buildarea_data()
    {
    }
};

typedef test_group<test_capi_buildarea_data> group;
typedef group::object object;

group test_capi_buildarea_group("capi::GEOSBuildArea");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION(LINESTRING(0 0,0 1,1 1),LINESTRING(1 1,1 0,0 0))");
    geom2_ = GEOSBuildArea(geom1_);
    ensure_equals(toWKT(geom2_), std::string("POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))"));
}
} // namespace tut
