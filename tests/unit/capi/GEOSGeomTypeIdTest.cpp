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
    geom3_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(nullptr != geom3_);

    ensure_equals(GEOS_POINT, GEOSGeomTypeId(geom1_));
    ensure_equals(GEOS_LINESTRING, GEOSGeomTypeId(geom2_));
    ensure_equals(GEOS_CIRCULARSTRING, GEOSGeomTypeId(geom3_));
}

} // namespace tut

