#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createlinestring_data : public capitest::utility {};

typedef test_group<test_geosgeom_createlinestring_data> group;
typedef group::object object;

group test_geosgeom_createlinestring("capi::GEOSGeom_createLineString");

template<>
template<>
void object::test<1>
()
{
    GEOSCoordSequence* seq = GEOSCoordSeq_create(3, 2);

    GEOSCoordSeq_setXY(seq, 0, 1, 2);
    GEOSCoordSeq_setXY(seq, 1, 4, 5);
    GEOSCoordSeq_setXY(seq, 2, 9, -2);

    GEOSGeometry* result = GEOSGeom_createLineString(seq);
    GEOSGeometry* expected = GEOSGeomFromWKT("LINESTRING (1 2, 4 5, 9 -2)");

    ensure_equals(GEOSEqualsExact(result, expected, 0), 1);

    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

} // namespace tut

