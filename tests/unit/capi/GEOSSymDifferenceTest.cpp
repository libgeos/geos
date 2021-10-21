#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geossymdifference_data : public capitest::utility {};

typedef test_group<test_geossymdifference_data> group;
typedef group::object object;

group test_geossymdifference("capi::GEOSSymDifference");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING(50 100, 50 200)");
    ensure(geom1_);
    geom2_ = fromWKT("LINESTRING(50 50, 50 150)");
    ensure(geom2_);
    geom3_ = GEOSSymDifference(geom1_, geom2_);

    ensure_equals("MULTILINESTRING ((50 150, 50 200), (50 50, 50 100))", toWKT(geom3_));
}

} // namespace tut

