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
struct test_capigeosconcavehull_data : public capitest::utility {
};

typedef test_group<test_capigeosconcavehull_data> group;
typedef group::object object;

group test_capigeosconcavehull_group("capi::GEOSConcaveHull");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("MULTIPOINT ((10 90), (10 10), (90 10), (90 90), (40 40), (60 30), (30 70), (40 60), (60 50), (60 72), (47 66), (90 60))");
    ensure(nullptr != input_);

    expected_ = GEOSGeomFromWKT("POLYGON ((30 70, 10 90, 60 72, 90 90, 90 60, 90 10, 60 30, 10 10, 40 40, 60 50, 47 66, 40 60, 30 70))");
    ensure(nullptr != expected_);

    GEOSGeometry* output = GEOSConcaveHull(input_, 0, 0);
    ensure(nullptr != output);
    ensure(0 == GEOSisEmpty(output));
    ensure_geometry_equals(output, expected_);
    GEOSGeom_destroy(output);
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("MULTIPOINT ((10 90), (10 10), (90 10), (90 90), (40 40), (60 30), (30 70), (40 60), (60 50), (60 72), (47 66), (90 60))");
    ensure(nullptr != input_);

    expected_ = GEOSGeomFromWKT("POLYGON ((30 70, 10 90, 60 72, 90 90, 90 60, 90 10, 60 30, 10 10, 40 40, 30 70))");
    ensure(nullptr != expected_);

    GEOSGeometry* output = GEOSConcaveHullByLength(input_, 50, 0);
    ensure(nullptr != output);
    ensure(0 == GEOSisEmpty(output));
    ensure_geometry_equals(output, expected_);
    GEOSGeom_destroy(output);
}

} // namespace tut
