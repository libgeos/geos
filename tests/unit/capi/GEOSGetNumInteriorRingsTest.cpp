#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgetnuminteriorrings_data : public capitest::utility {};

typedef test_group<test_geosgetnuminteriorrings_data> group;
typedef group::object object;

group test_getnuminteriorrings("capi::GEOSGetNumInteriorRings");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POLYGON ((1 -2, 9 -2, 9 5, 1 5, 1 -2))");

    ensure_equals(GEOSGetNumInteriorRings(input), 0);

    GEOSGeom_destroy(input);
}

template<>
template<>
void object::test<2>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("LINESTRING (3 8, 4 7)");

    ensure_equals(GEOSGetNumInteriorRings(input), -1);

    GEOSGeom_destroy(input);
}

template<>
template<>
void object::test<3>()
{
    input_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 10 10, 20 0), (20 0, 0 0)), (8 8, 9 9, 9 8, 8 8))");
    ensure(input_ != nullptr);

    ensure_equals(GEOSGetNumInteriorRings(input_), 1);
}

} // namespace tut

