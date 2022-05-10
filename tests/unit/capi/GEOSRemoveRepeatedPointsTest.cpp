// Test Suite for C-API RemoveRepeatedPoits

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capiremoverepeatedpoints_data : public capitest::utility {
    test_capiremoverepeatedpoints_data() {
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setOutputDimension(wktw_, 3);
    }
};

typedef test_group<test_capiremoverepeatedpoints_data> group;
typedef group::object object;

group test_capiremoverepeatedpoints_group("capi::GEOSRemoveRepeatedPoints");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 1, 0 10, 10 10, 10 0, 9 0, 1 0, 0 0))");
    geom2_ = GEOSRemoveRepeatedPoints(geom1_, 3.0);
    expected_ = GEOSGeomFromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))");
    ensure_geometry_equals(geom2_, expected_);
}



} // namespace tut
