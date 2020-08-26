#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosdifference_data : public capitest::utility {};

typedef test_group<test_geosdifference_data> group;
typedef group::object object;

group test_geosdifference("capi::GEOSDifference");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* a = GEOSGeomFromWKT("LINESTRING (2 8, 10 8)");
    GEOSGeometry* b = GEOSGeomFromWKT("LINESTRING (4 8, 6 8)");

    ensure(a);
    ensure(b);

    GEOSSetSRID(a, 4326);

    GEOSGeometry* result = GEOSDifference(a, b);

    ensure(result);

    ensure_geometry_equals(result, "MULTILINESTRING ((6 8, 10 8), (2 8, 4 8))");
    ensure_equals(GEOSGetSRID(a), GEOSGetSRID(result));

    GEOSGeom_destroy(a);
    GEOSGeom_destroy(b);
    GEOSGeom_destroy(result);
}

} // namespace tut

