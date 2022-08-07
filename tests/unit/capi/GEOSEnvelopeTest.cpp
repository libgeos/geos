#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosenvelope_data : public capitest::utility {};

typedef test_group<test_geosenvelope_data> group;
typedef group::object object;

group test_geosenvelope("capi::GEOSEnvelope");

// non-degenerate input
template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* input = GEOSGeomFromWKT("LINESTRING (1 2, 4 5, 9 -2)");
    GEOSGeometry* result = GEOSEnvelope(input);
    GEOSGeometry* expected = GEOSGeomFromWKT("POLYGON ((1 -2, 9 -2, 9 5, 1 5, 1 -2))");

    ensure_equals(GEOSEqualsExact(result, expected, 0), 1);

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

// point input
template<>
template<>
void object::test<2>
()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POINT (3 8)");
    GEOSGeometry* result = GEOSEnvelope(input);

    GEOSGeometry* expected = GEOSGeomFromWKT("POINT (3 8)");

    ensure_geometry_equals(result, expected);

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

// empty point input
template<>
template<>
void object::test<3>
()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POINT EMPTY");
    GEOSGeometry* result = GEOSEnvelope(input);

    GEOSGeometry* expected = GEOSGeomFromWKT("POINT EMPTY");

    ensure_geometry_equals(result, expected);

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

// empty polygon input
template<>
template<>
void object::test<4>
()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POLYGON EMPTY");
    GEOSGeometry* result = GEOSEnvelope(input);

    GEOSGeometry* expected = GEOSGeomFromWKT("POINT EMPTY");

    ensure_geometry_equals(result, expected);

    GEOSGeom_destroy(input);
    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

} // namespace tut

