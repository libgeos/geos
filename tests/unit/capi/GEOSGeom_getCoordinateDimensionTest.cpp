#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_getcoordinatedimension_data : public capitest::utility {};

typedef test_group<test_geosgeom_getcoordinatedimension_data> group;
typedef group::object object;

group test_geosgeom_getcoordinatedimension("capi::GEOSGeom_getCoordinateDimension");

template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* point = GEOSGeomFromWKT("POINT (4 2 7)");
    GEOSGeometry* line = GEOSGeomFromWKT("LINESTRING (4 2 7, 8 2 9)");
    GEOSGeometry* poly = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");

    ensure_equals(GEOSGeom_getCoordinateDimension(point), 3);
    ensure_equals(GEOSGeom_getCoordinateDimension(line), 3);
    ensure_equals(GEOSGeom_getCoordinateDimension(poly), 2);

    GEOSGeom_destroy(point);
    GEOSGeom_destroy(line);
    GEOSGeom_destroy(poly);
}

} // namespace tut

