#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosisvalid_data : public capitest::utility {};

typedef test_group<test_geosisvalid_data> group;
typedef group::object object;

group test_geosisvalid("capi::GEOSisValid");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("LINESTRING (1 2, 4 5, 9 -2)");

    ensure_equals(1, GEOSisValid(input));

    GEOSGeom_destroy(input);
}

template<>
template<>
void object::test<2>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 0 1, 1 1, 0 0))");

    ensure_equals(0, GEOSisValid(input));

    GEOSGeom_destroy(input);
}


} // namespace tut

