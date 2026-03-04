//
// Test Suite for C-API GEOSEquals

#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos_c.h>
// std
#include <cstring>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosequalsexact_data : public capitest::utility {};

typedef test_group<test_capigeosequalsexact_data> group;
typedef group::object object;

group test_capigeosequalsexact_group("capi::GEOSEqualsExact");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    set_test_name("almost-identical CircularStrings");

    geom1_ = GEOSGeomFromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = GEOSGeomFromWKT("CIRCULARSTRING (0 0, 1 1.001, 2 0)");

    ensure_equals(GEOSEqualsExact(geom1_, geom2_, 0), 0);
    ensure_equals(GEOSEqualsExact(geom1_, geom2_, 1e-6), 0);
    ensure_equals(GEOSEqualsExact(geom1_, geom2_, 1e-3), 1);
}

} // namespace tut

