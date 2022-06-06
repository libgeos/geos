//
// Test Suite for C-API GEOSConcaveHull

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
struct test_capigeoshullofpolygons_data : public capitest::utility {
};

typedef test_group<test_capigeoshullofpolygons_data> group;
typedef group::object object;

group test_capigeoshullofpolygons_group("capi::GEOSConcaveHullOfPolygons");

//
// Test Cases
//


template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure(nullptr != input_);
    geom1_ = GEOSConcaveHullOfPolygons(input_, 0.7, false, false);
    ensure(nullptr != geom1_);
    expected_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure_geometry_equals(geom1_, expected_);
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0, 0 1, 1 1, 1 0, 0 0)))");
    ensure(nullptr != input_);
    geom1_ = GEOSConcaveHullOfPolygons(input_, 0.7, false, false);
    ensure(nullptr != geom1_);
    expected_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0, 0 1, 1 1, 1 0, 0 0)))");
    ensure_geometry_equals(geom1_, expected_);
}


} // namespace tut

