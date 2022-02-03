// Test Suite for C-API GEOSGeom_createEnvelope

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "capi_test_utils.h"

namespace tut {
struct test_capigeosgeom_createenvelope : public capitest::utility {};

typedef test_group<test_capigeosgeom_createenvelope> group;
typedef group::object object;

group test_capigeosgeom_createenvelope_group("capi::GEOSGeom_createEnvelope");

template <>
template <>
void object::test<1>() {
    GEOSGeometry* expected = GEOSGeomFromWKT("POLYGON ((0 0, 0 2, 1 2, 1 0, 0 0))");
    GEOSGeometry* geom = GEOSGeom_createEnvelope(0, 0, 1, 2);

    ensure_equals(GEOSEqualsExact(geom, expected, 0), 1);

    GEOSGeom_destroy(expected);
    GEOSGeom_destroy(geom);
}

}  // namespace tut
