#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createemptynurbscurve_data : public capitest::utility {};

typedef test_group<test_geosgeom_createemptynurbscurve_data> group;
typedef group::object object;

group test_geosgeom_createemptynurbscurve("capi::GEOSGeom_createEmptyNurbsCurve");

template<>
template<>
void object::test<1>()
{
    result_ = GEOSGeom_createEmptyNurbsCurve();
    ensure(GEOSisEmpty(result_));
    ensure(!GEOSHasZ(result_));
    ensure(!GEOSHasM(result_));
    ensure_equals(GEOSGeomTypeId(result_), GEOS_NURBSCURVE);
}

} // namespace tut
