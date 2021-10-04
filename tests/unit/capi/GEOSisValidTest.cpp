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


// Unclosed polygon
template<>
template<>
void object::test<3>()
{
    GEOSCoordSequence* shell_seq = GEOSCoordSeq_create(4, 2);
    double shell_coords[] = {0,0, 0,10, 10,10, 10,0};
    for (unsigned int i = 0; i < 4; i++) {
        GEOSCoordSeq_setXY(shell_seq, i, shell_coords[2*i], shell_coords[2*i+1]);
    }

    // Unclosed ring fails in construction
    // Linear ring takes ownership of coordinate sequence and
    // frees it when construction fails
    GEOSGeometry* shell = GEOSGeom_createLinearRing(shell_seq);
    ensure(shell == nullptr);
    // So we end up handing nullptr into create polygon
    GEOSGeometry* polygon = GEOSGeom_createPolygon(shell, nullptr, 0);
    ensure(polygon == nullptr);
    // Which also returns nullptr and that goes into isvalid
    char isvalid = GEOSisValid(polygon);
    // Which causes an exception that we catch
    ensure_equals(2, isvalid);
}

} // namespace tut
