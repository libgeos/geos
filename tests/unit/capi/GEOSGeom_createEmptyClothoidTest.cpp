#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createemptyclothoid_data : public capitest::utility {};

typedef test_group<test_geosgeom_createemptyclothoid_data> group;
typedef group::object object;

group test_geosgeom_createemptyclothoid("capi::GEOSGeom_createEmptyClothoid");

template<>
template<>
void object::test<1>()
{
    result_ = GEOSGeom_createEmptyClothoid();
    ensure(GEOSisEmpty(result_));
    ensure(!GEOSHasZ(result_));
    ensure(!GEOSHasM(result_));
    ensure_equals(GEOSGeomTypeId(result_), GEOS_CLOTHOID);
}

} // namespace tut
