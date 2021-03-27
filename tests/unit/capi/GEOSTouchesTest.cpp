#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geostouches_data : public capitest::utility {};

typedef test_group<test_geostouches_data> group;
typedef group::object object;

group test_geostouches("capi::GEOSTouches");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* a = GEOSGeomFromWKT("LINESTRING (-122.440803 47.25315, -122.4401056 47.25322)");
    GEOSGeometry* b = GEOSGeomFromWKT("LINESTRING (-122.4401056 47.25322, -122.439993 47.25271)");
    GEOSGeometry* c = GEOSGeomFromWKT("LINESTRING (-122.4394243 47.25331, -122.4392044 47.25241)");

    ensure(a);
    ensure(b);
    ensure(c);

    ensure_equals(1, GEOSTouches(a, b));
    ensure_equals(1, GEOSTouches(b, a));
    ensure_equals(0, GEOSTouches(a, c));
    ensure_equals(0, GEOSTouches(c, a));
    ensure_equals(0, GEOSTouches(b, c));
    ensure_equals(0, GEOSTouches(c, b));

    GEOSGeom_destroy(a);
    GEOSGeom_destroy(b);
    GEOSGeom_destroy(c);
}

} // namespace tut

