#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosunion_data : public capitest::utility {};

typedef test_group<test_geosunion_data> group;
typedef group::object object;

group test_geosunion("capi::GEOSUnion");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* a = GEOSGeomFromWKT("POINT (2 8)");
    GEOSGeometry* b = GEOSGeomFromWKT("POINT (3 9)");

    ensure(a);
    ensure(b);

    GEOSSetSRID(a, 4326);

    GEOSGeometry* result = GEOSUnion(a, b);
    GEOSGeometry* expected = GEOSGeomFromWKT("MULTIPOINT (2 8, 3 9)");

    ensure(result);
    ensure(expected);

    ensure_equals(GEOSEqualsExact(result, expected, 0), 1);
    ensure_equals(GEOSGetSRID(a), GEOSGetSRID(result));

    GEOSGeom_destroy(a);
    GEOSGeom_destroy(b);
    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

} // namespace tut

