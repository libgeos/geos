//
// Test Suite for C-API GEOSFrechetDistance

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cmath>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosfrechetdistance_data : public capitest::utility {
    test_capigeosfrechetdistance_data() {
        GEOSWKTWriter_setTrim(wktw_, 1);
    }
};

typedef test_group<test_capigeosfrechetdistance_data> group;
typedef group::object object;

group test_capigeosfrechetdistance_group("capi::GEOSFrechetDistance");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 100 0)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (0 0, 50 50, 100 0)");

    double dist;
    int ret = GEOSFrechetDistance(geom1_, geom2_, &dist);

    ensure_equals(ret, 1);
    ensure_distance(dist, 70.7106781186548, 1e-12);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 100 0)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (0 0, 50 50, 100 0)");

    double dist;
    int ret = GEOSFrechetDistanceDensify(geom1_, geom2_, 0.5, &dist);

    ensure_equals(ret, 1);
    ensure_distance(dist, 50., 1e-12);
}

} // namespace tut
