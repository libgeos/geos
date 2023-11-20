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
    geom1_ = fromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");
    ensure_equals(GEOSGeom_getCoordinateDimension(geom1_), 2);

    geom2_ = fromWKT("POINT (4 2 7)");
    ensure_equals(GEOSGeom_getCoordinateDimension(geom2_), 3);

    geom3_ = fromWKT("LINESTRING (4 2 7 1, 8 2 9 5)");
    ensure_equals(GEOSGeom_getCoordinateDimension(geom3_), 4);
}

} // namespace tut

