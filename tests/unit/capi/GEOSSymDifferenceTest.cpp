#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geossymdifference_data : public capitest::utility {};

typedef test_group<test_geossymdifference_data> group;
typedef group::object object;

group test_geossymdifference("capi::GEOSSymDifference");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING(50 100, 50 200)");
    ensure(geom1_);
    geom2_ = fromWKT("LINESTRING(50 50, 50 150)");
    ensure(geom2_);
    geom3_ = GEOSSymDifference(geom1_, geom2_);

    ensure_equals("MULTILINESTRING ((50 150, 50 200), (50 50, 50 100))", toWKT(geom3_));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("curved inputs");

    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    result_ = GEOSSymDifference(geom1_, geom2_);
    ensure(result_);

    expected_ = fromWKT("MULTICURVE (CIRCULARSTRING (1.7071067812 0.7071067812, 1.9238795325 0.3826834324, 2 0), CIRCULARSTRING (0 0, 0.6173165676 0.9238795325, 1.7071067812 0.7071067812), (1.7071067812 0.7071067812, 2 1), (1 0, 1.7071067812 0.7071067812))");
    ensure_geometry_equals(result_, expected_, 1e-8);
}

} // namespace tut

