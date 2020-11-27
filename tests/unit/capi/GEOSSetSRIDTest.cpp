#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geossetsrid_data : public capitest::utility {};

typedef test_group<test_geossetsrid_data> group;
typedef group::object object;

group test_geossetsrid("capi::GEOSSetSRID");

template<>
template<>
void object::test<1>
()
{
    int expected_srid = 1234;
    GEOSGeometry* input = GEOSGeomFromWKT("LINESTRING (1 2, 4 5, 9 -2)");
    GEOSSetSRID(input, expected_srid);
    ensure_equals(GEOSGetSRID(input), expected_srid);

    GEOSGeom_destroy(input);
}

} // namespace tut

