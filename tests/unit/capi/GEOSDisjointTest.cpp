#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosdisjoint_data : public capitest::utility {};

typedef test_group<test_geosdisjoint_data> group;
typedef group::object object;

group test_geosdisjoint("capi::GEOSDisjoint");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* a = GEOSGeomFromWKT("POLYGON ((-121.915 47.39, -122.64 46.995, -121.739 46.308, -121.168 46.777, -120.981 47.316, -121.409 47.413, -121.915 47.39))");
    GEOSGeometry* b = GEOSGeomFromWKT("POLYGON ((-120.794 46.664, -121.541 46.995, -122.2 46.536, -121.937 45.89, -120.959 46.096, -120.794 46.664))");
    GEOSGeometry* c = GEOSGeomFromWKT("POLYGON ((-120.541 47.376, -120.695 47.047, -119.794 46.83, -119.586 47.331, -120.102 47.509, -120.541 47.376))");

    ensure(a);
    ensure(b);
    ensure(c);

    ensure_equals(0, GEOSDisjoint(a, b));
    ensure_equals(1, GEOSDisjoint(a, c));
    ensure_equals(1, GEOSDisjoint(b, c));

    GEOSGeom_destroy(a);
    GEOSGeom_destroy(b);
    GEOSGeom_destroy(c);
}

} // namespace tut

