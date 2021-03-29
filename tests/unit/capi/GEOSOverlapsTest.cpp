#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosoverlaps_data : public capitest::utility {};

typedef test_group<test_geosoverlaps_data> group;
typedef group::object object;

group test_geosoverlaps("capi::GEOSOverlaps");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))");
    ensure(nullptr != geom1_);
    geom2_ = fromWKT("POLYGON ((5 5, 5 15, 15 15, 15 5, 5 5))");
    ensure(nullptr != geom2_);
    geom3_ = fromWKT("POLYGON ((20 20, 20 30, 30 30, 30 20, 20 20))");
    ensure(nullptr != geom3_);

    ensure_equals(1, GEOSOverlaps(geom1_, geom2_));
    ensure_equals(1, GEOSOverlaps(geom2_, geom1_));
    ensure_equals(0, GEOSOverlaps(geom1_, geom3_));
    ensure_equals(0, GEOSOverlaps(geom3_, geom1_));
    ensure_equals(0, GEOSOverlaps(geom2_, geom3_));
    ensure_equals(0, GEOSOverlaps(geom3_, geom2_));
}

} // namespace tut

