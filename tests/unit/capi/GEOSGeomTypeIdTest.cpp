#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeomtypeid_data : public capitest::utility {};

typedef test_group<test_geosgeomtypeid_data> group;
typedef group::object object;

group test_geosgeomtypeid("capi::GEOSGeomTypeId");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POINT (1 2)");
    ensure(nullptr != geom1_);
    geom2_ = fromWKT("LINESTRING (1 2, 3 4)");
    ensure(nullptr != geom2_);

    ensure_equals(0, GEOSGeomTypeId(geom1_));
    ensure_equals(1, GEOSGeomTypeId(geom2_));
}

} // namespace tut

