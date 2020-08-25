#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosboundary_data : public capitest::utility {};

typedef test_group<test_geosboundary_data> group;
typedef group::object object;

group test_geosboundary("capi::GEOSBoundary");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
    GEOSSetSRID(input, 3857);

    GEOSGeometry* result = GEOSBoundary(input);
    GEOSGeometry* expected = GEOSGeomFromWKT("MULTILINESTRING ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");

    ensure_equals(GEOSEqualsExact(result, expected, 0), 1);
    ensure_equals(GEOSGetSRID(input), GEOSGetSRID(result));

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

} // namespace tut

