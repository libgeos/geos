#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosisvalidreason_data : public capitest::utility {};

typedef test_group<test_geosisvalidreason_data> group;
typedef group::object object;

group test_geosisvalidreason("capi::GEOSisValidReason");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("LINESTRING (1 2, 4 5, 9 -2)");

    char* reason = GEOSisValidReason(input);

    ensure_equals(std::string(reason), "Valid Geometry");

    GEOSGeom_destroy(input);
    GEOSFree(reason);
}

template<>
template<>
void object::test<2>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 0 1, 1 1, 0 0))");

    char* reason = GEOSisValidReason(input);

    ensure_equals(std::string(reason), "Self-intersection[0.5 0.5]");

    GEOSGeom_destroy(input);
    GEOSFree(reason);
}


} // namespace tut

