#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosdisjoint_data : public capitest::utility {};

typedef test_group<test_geosdisjoint_data> group;
typedef group::object object;

group test_geosdisjoint("capi::GEOSDisjoint");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POINT (0 0)");
    ensure(geom1_);
    geom2_ = fromWKT("LINESTRING(2 0, 0 2)");
    ensure(geom2_);
    geom3_ = fromWKT("LINESTRING(0 0, 0 2)");
    ensure(geom3_);

    ensure_equals(1, GEOSDisjoint(geom1_, geom2_));
    ensure_equals(0, GEOSDisjoint(geom1_, geom3_));
}

} // namespace tut

