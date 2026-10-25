#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createemptyspiralcurve_data : public capitest::utility {};

typedef test_group<test_geosgeom_createemptyspiralcurve_data> group;
typedef group::object object;

group test_geosgeom_createemptyspiralcurve("capi::GEOSGeom_createEmptySpiralCurve");

template<>
template<>
void object::test<1>()
{
    result_ = GEOSGeom_createEmptySpiralCurve();
    ensure(GEOSisEmpty(result_));
    ensure(!GEOSHasZ(result_));
    ensure(!GEOSHasM(result_));
    ensure_equals(GEOSGeomTypeId(result_), GEOS_SPIRALCURVE);
}

} // namespace tut
