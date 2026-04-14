#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosdifference_data : public capitest::utility {};

typedef test_group<test_geosdifference_data> group;
typedef group::object object;

group test_geosdifference("capi::GEOSDifference");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING (2 8, 10 8)");
    geom2_ = fromWKT("LINESTRING (4 8, 6 8)");

    GEOSSetSRID(geom1_, 4326);

    result_ = GEOSDifference(geom1_, geom2_);
    ensure(result_);

    ensure_geometry_equals(result_, "MULTILINESTRING ((6 8, 10 8), (2 8, 4 8))");
    ensure_equals(GEOSGetSRID(geom1_), GEOSGetSRID(result_));
}

/**
* Mixed GeometryCollection types permitted at a high-level
*/
template<>
template<>
void object::test<2>() {
    GEOSGeometry* a = GEOSGeomFromWKT("GEOMETRYCOLLECTION (POINT (51 -1), LINESTRING (52 -1, 49 2))");
    GEOSGeometry* b = GEOSGeomFromWKT("POINT (2 3)");

    ensure(a);
    ensure(b);

    GEOSGeometry* ab = GEOSDifference(a, b);
    GEOSGeometry* ba = GEOSDifference(b, a);

    ensure(ab);
    ensure(ba);

    ensure_geometry_equals(ab, a);
    ensure_geometry_equals(ba, b);

    GEOSGeom_destroy(a);
    GEOSGeom_destroy(b);
    GEOSGeom_destroy(ab);
    GEOSGeom_destroy(ba);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("curved inputs");

    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    ensure(geom1_);
    ensure(geom2_);

    result_ = GEOSDifference(geom1_, geom2_);
    ensure(result_);

    expected_ = fromWKT("MULTICURVE (CIRCULARSTRING (1.7071067812 0.7071067812, 1.9238795325 0.3826834324, 2 0), CIRCULARSTRING (0 0, 0.6173165676 0.9238795325, 1.7071067812 0.7071067812))");

    ensure_geometry_equals(result_, expected_, 1e-8);
}

} // namespace tut

