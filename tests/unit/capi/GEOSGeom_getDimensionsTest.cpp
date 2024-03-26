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
    geom1_ = fromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");
    ensure_equals(GEOSGeom_getDimensions(geom1_), 2);

    geom3_ = fromWKT("LINESTRING (4 2 7 1, 8 2 9 5)");
    ensure_equals(GEOSGeom_getDimensions(geom3_), 1);

    geom2_ = fromWKT("POINT (4 2 7)");
    ensure_equals(GEOSGeom_getDimensions(geom2_), 0);
}

template<>
template<>
void object::test<2>()
{
    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure_equals(GEOSGeom_getDimensions(input_), 1);
}

} // namespace tut

