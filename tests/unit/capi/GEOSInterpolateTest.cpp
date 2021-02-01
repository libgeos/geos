// Test Suite for C-API LineString interpolate functions

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
struct test_capiinterpolate_data : public capitest::utility {};

typedef test_group<test_capiinterpolate_data> group;
typedef group::object object;

group test_capiinterpolate_group("capi::GEOSInterpolate");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    GEOSGeometry* geom2 = GEOSInterpolate(geom1_, 1);
    ensure_equals(GEOSisEmpty(geom2), 1);
    GEOSGeom_destroy(geom2);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION EMPTY");
    GEOSGeometry* geom2 = GEOSInterpolate(geom1_, 1);
    ensure_equals(GEOSisEmpty(geom2), 1);
    GEOSGeom_destroy(geom2);
}

} // namespace tut
