#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoscrosses_data : public capitest::utility {};

typedef test_group<test_geoscrosses_data> group;
typedef group::object object;

group test_geoscrosses("capi::GEOSCrosses");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING (1 1, 10 10)");
    geom2_ = fromWKT("LINESTRING (10 1, 1 10)");
    geom3_ = fromWKT("LINESTRING (20 20, 30 30)");

    ensure_equals(1, GEOSCrosses(geom1_, geom2_));
    ensure_equals(1, GEOSCrosses(geom2_, geom1_));
    ensure_equals(0, GEOSCrosses(geom1_, geom3_));
    ensure_equals(0, GEOSCrosses(geom3_, geom1_));
    ensure_equals(0, GEOSCrosses(geom2_, geom3_));
    ensure_equals(0, GEOSCrosses(geom3_, geom2_));
}

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals("curved geometry not supported", GEOSCrosses(geom1_, geom2_), 2);
    ensure_equals("curved geometry not supported", GEOSCrosses(geom2_, geom1_), 2);
}

} // namespace tut

