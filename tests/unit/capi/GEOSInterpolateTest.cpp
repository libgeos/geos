// Test Suite for C-API LineString interpolate functions

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

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

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
    result_ = GEOSInterpolateNormalized(geom1_, 0.5);
    expected_ = GEOSGeomFromWKT("POINT (5 0)");

    ensure_geometry_equals_identical(result_, expected_);
}

// ensure NaN argument does not crash
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");
    result_ = GEOSInterpolate(geom1_, std::numeric_limits<double>::quiet_NaN());
}

template<>
template<>
void object::test<6>
()
{
    set_test_name("Z/M interpolated");

    geom1_ = fromWKT("LINESTRING ZM (0 0 6 8, 10 0 12 16)");
    result_ = GEOSInterpolate(geom1_, 5);
    expected_ = fromWKT("POINT ZM (5 0 9 12)");

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<7>
()
{
    set_test_name("Empty LINESTRING M");

    geom1_ = fromWKT("LINESTRING M EMPTY");
    result_ = GEOSInterpolate(geom1_, 5);
    expected_ = fromWKT("POINT M EMPTY");

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<8>
()
{
    set_test_name("Empty LINESTRING Z");

    geom1_ = fromWKT("LINESTRING Z EMPTY");
    result_ = GEOSInterpolate(geom1_, 5);
    expected_ = fromWKT("POINT Z EMPTY");

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<9>
()
{
    set_test_name("Empty LINESTRING ZM");

    geom1_ = fromWKT("LINESTRING ZM EMPTY");
    result_ = GEOSInterpolate(geom1_, 5);
    expected_ = fromWKT("POINT ZM EMPTY");

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<10>
()
{
    set_test_name("Non-LineString input");

    input_ = fromWKT("POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))");
    ensure(input_);

    result_ = GEOSInterpolate(input_, 5);

    ensure(result_ == nullptr);
}

} // namespace tut
