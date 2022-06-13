//
// Test Suite for C-API GEOSPolygonHullSimplify

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
struct test_capigeospolygonhull_data : public capitest::utility {
    GEOSParams *params_;
    test_capigeospolygonhull_data() {
        params_ = GEOSParams_create();
        GEOSParams_setParamInteger(params_, "hull_location", GEOS_POLYHULL_OUTSIDE);
        GEOSParams_setParamInteger(params_, "hull_ratio", GEOS_POLYHULL_VERTEX_RATIO);
    }
    ~test_capigeospolygonhull_data() {
        GEOSParams_destroy(params_);
    }
};

typedef test_group<test_capigeospolygonhull_data> group;
typedef group::object object;

group test_capigeospolygonhull_group("capi::GEOSPolygonHullSimplify");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("POLYGON ((10 90, 40 60, 20 40, 40 20, 70 50, 40 30, 30 40, 60 70, 50 90, 90 90, 90 10, 10 10, 10 90))");
    ensure(nullptr != input_);
    geom1_ = GEOSPolygonHullSimplify(input_, 0.5, params_);
    ensure(nullptr != geom1_);
    expected_ = GEOSGeomFromWKT("POLYGON ((10 90, 50 90, 90 90, 90 10, 10 10, 10 90))");
    ensure_geometry_equals(geom1_, expected_);
    ensure(0 == GEOSisEmpty(geom1_));
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("POLYGON ((10 90, 40 60, 20 40, 40 20, 70 50, 40 30, 30 40, 60 70, 50 90, 90 90, 90 10, 10 10, 10 90))");
    ensure(nullptr != input_);
    geom1_ = GEOSPolygonHullSimplify(input_, 0.7, params_);
    ensure(nullptr != geom1_);
    expected_ = GEOSGeomFromWKT("POLYGON ((10 90, 40 60, 30 40, 60 70, 50 90, 90 90, 90 10, 10 10, 10 90))");
    ensure_geometry_equals(geom1_, expected_);
    ensure(0 == GEOSisEmpty(geom1_));
}

template<>
template<>
void object::test<3>()
{
    input_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure(nullptr != input_);
    geom1_ = GEOSPolygonHullSimplify(input_, 0.7, params_);
    ensure(nullptr != geom1_);
    expected_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure_geometry_equals(geom1_, expected_);
}

template<>
template<>
void object::test<4>()
{
    input_ = GEOSGeomFromWKT("POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))");
    ensure(nullptr != input_);
    geom1_ = GEOSPolygonHullSimplify(input_, 0.7, params_);
    ensure(nullptr != geom1_);
    expected_ = GEOSGeomFromWKT("POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))");
    ensure_geometry_equals(geom1_, expected_);
}


} // namespace tut

