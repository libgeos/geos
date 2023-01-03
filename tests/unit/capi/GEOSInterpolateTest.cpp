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
    result_ = GEOSInterpolate(geom1_, 1);
    ensure_equals(GEOSisEmpty(result_), 1);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION EMPTY");
    result_ = GEOSInterpolate(geom1_, 1);
    ensure_equals(GEOSisEmpty(result_), 1);
}

template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
    result_ = GEOSInterpolate(geom1_, 0.5);
    expected_ = GEOSGeomFromWKT("POINT (0.5 0)");

    ensure_geometry_equals(result_, expected_);
}

template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
    result_ = GEOSInterpolateNormalized(geom1_, 0.5);
    expected_ = GEOSGeomFromWKT("POINT (5 0)");

    ensure_geometry_equals(result_, expected_);
}

} // namespace tut
