#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosunionprec_data : public capitest::utility {};

typedef test_group<test_geosunionprec_data> group;
typedef group::object object;

group test_geosunionprec("capi::GEOSUnionPrec");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* a = GEOSGeomFromWKT("POINT (1.9 8.2)");
    GEOSGeometry* b = GEOSGeomFromWKT("POINT (4.1 9.8)");

    ensure(a);
    ensure(b);

    GEOSSetSRID(a, 4326);

    GEOSGeometry* result = GEOSUnionPrec(a, b, 2);
    GEOSGeometry* expected = GEOSGeomFromWKT("MULTIPOINT (2 8, 4 10)");

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

