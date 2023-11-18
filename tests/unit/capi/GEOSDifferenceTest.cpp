#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosdifference_data : public capitest::utility {};

typedef test_group<test_geosdifference_data> group;
typedef group::object object;

group test_geosdifference("capi::GEOSDifference");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING (2 8, 10 8)");
    geom2_ = fromWKT("LINESTRING (4 8, 6 8)");

    GEOSSetSRID(geom1_, 4326);

    result_ = GEOSDifference(geom1_, geom2_);
    ensure(result_);

    ensure_geometry_equals(result_, "MULTILINESTRING ((6 8, 10 8), (2 8, 4 8))");
    ensure_equals(GEOSGetSRID(geom1_), GEOSGetSRID(result_));
}

} // namespace tut

