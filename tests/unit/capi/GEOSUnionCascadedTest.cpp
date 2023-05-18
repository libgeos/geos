#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosunioncascaded_data : public capitest::utility {};

typedef test_group<test_geosunioncascaded_data> group;
typedef group::object object;

group test_geosunioncascaded("capi::GEOSUnionCascaded");

template<>
template<>
void object::test<1>()
{
    geom1_ = GEOSGeomFromWKT("POINT (2 8)");
    ensure(nullptr != geom1_);
    geom2_ = GEOSGeomFromWKT("POINT (3 9)");
    ensure(nullptr != geom2_);

    geom3_ = GEOSUnion(geom1_, geom2_);
    ensure(nullptr != geom3_);

     ensure_equals("MULTIPOINT ((2 8), (3 9))", toWKT(geom3_));
}

} // namespace tut

