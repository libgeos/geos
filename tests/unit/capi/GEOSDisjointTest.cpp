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
    geom2_ = fromWKT("LINESTRING(2 0, 0 2)");
    geom3_ = fromWKT("LINESTRING(0 0, 0 2)");

    ensure_equals(1, GEOSDisjoint(geom1_, geom2_));
    ensure_equals(0, GEOSDisjoint(geom1_, geom3_));
}

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals("curved geometry not supported", GEOSDisjoint(geom1_, geom2_), 2);
    ensure_equals("curved geometry not supported", GEOSDisjoint(geom2_, geom1_), 2);
}

} // namespace tut

