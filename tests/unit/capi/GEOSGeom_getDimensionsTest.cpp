#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_getdimensions_data : public capitest::utility {};

typedef test_group<test_geosgeom_getdimensions_data> group;
typedef group::object object;

group test_geosgeom_getdimensions("capi::GEOSGeom_getDimensions");

template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* point = GEOSGeomFromWKT("POINT (4 2 7)");
    GEOSGeometry* line = GEOSGeomFromWKT("LINESTRING (4 2 7, 8 2 9)");
    GEOSGeometry* poly = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");

    ensure_equals(GEOSGeom_getDimensions(point), 0);
    ensure_equals(GEOSGeom_getDimensions(line), 1);
    ensure_equals(GEOSGeom_getDimensions(poly), 2);

    GEOSGeom_destroy(point);
    GEOSGeom_destroy(line);
    GEOSGeom_destroy(poly);
}

} // namespace tut

