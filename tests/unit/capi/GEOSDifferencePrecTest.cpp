#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosdifferenceprec_data : public capitest::utility {};

typedef test_group<test_geosdifferenceprec_data> group;
typedef group::object object;

group test_geosdifferenceprec("capi::GEOSDifferencePrec");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING (2 8, 10 8)");
    geom2_ = fromWKT("LINESTRING (3.9 8.1, 6.1 7.9)");
    GEOSSetSRID(geom1_, 4326);

    result_ = GEOSDifferencePrec(geom1_, geom2_, 2);
    ensure(result_);

    ensure_geometry_equals(result_, "MULTILINESTRING ((6 8, 10 8), (2 8, 4 8))");
    ensure_equals(GEOSGetSRID(geom1_), GEOSGetSRID(result_));
}

} // namespace tut

