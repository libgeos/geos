#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_getcoordseq_data : public capitest::utility {};

typedef test_group<test_geosgeom_getcoordseq_data> group;
typedef group::object object;

group test_geosgeom_getcoordseq("capi::GEOSGeom_getCoordSeq");

template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* input = GEOSGeomFromWKT("LINESTRING (1 2, 4 5, 9 -2)");
    const GEOSCoordSequence* seq = GEOSGeom_getCoordSeq(input);

    double x = -1;
    double y = -1;
    GEOSCoordSeq_getXY(seq,  2, &x, &y);

    ensure_equals(x, 9);
    ensure_equals(y, -2);

    GEOSGeom_destroy(input);
}

template<>
template<>
void object::test<2>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POLYGON ((1 1, 2 1, 2 2, 1 1))");
    const GEOSCoordSequence* seq = GEOSGeom_getCoordSeq(input);

    ensure(seq == nullptr); // can't get seq from Polygon

    GEOSGeom_destroy(input);
}

} // namespace tut

