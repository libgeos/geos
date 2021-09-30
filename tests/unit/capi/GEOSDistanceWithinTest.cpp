//
// Test Suite for C-API GEOSDistance

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/constants.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <math.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosdistancewithin_data : public capitest::utility {
    test_capigeosdistancewithin_data() {
        GEOSWKTWriter_setTrim(wktw_, 1);
    }
};

typedef test_group<test_capigeosdistancewithin_data> group;
typedef group::object object;

group test_capigeosdistancewithin_group("capi::GEOSDistanceWithin");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(10 10)");
    geom2_ = GEOSGeomFromWKT("POINT(3 6)");

    double dist = 8.1;
    char ret = GEOSDistanceWithin(geom1_, geom2_, dist);
    ensure_equals(ret, 1);

    ret = GEOSDistanceWithin(geom1_, geom2_, dist-0.1);
    ensure_equals(ret, 0);
}

} // namespace tut

