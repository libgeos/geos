#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgetgeometryn_data : public capitest::utility {};

typedef test_group<test_geosgetgeometryn_data> group;
typedef group::object object;

group test_geosgetgeometryn("capi::GEOSGetGeometryN");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("MULTIPOINT ((1 1), (2 2), (3 3))");
    ensure(nullptr != geom1_);

    const GEOSGeometry* result = GEOSGetGeometryN(geom1_, 0);
    ensure(nullptr != result);
    ensure_equals("POINT (1 1)", toWKT(result));
    
    result = GEOSGetGeometryN(geom1_, 2);
    ensure(nullptr != result);
    ensure_equals("POINT (3 3)", toWKT(result));

    ensure(GEOSGetGeometryN(geom1_, -1) == nullptr);
}

template<>
template<>
void object::test<2>()
{
    input_ = fromWKT("MULTICURVE ((0 0, 1 1), CIRCULARSTRING (1 1, 2 0, 3 1))");
    ensure(input_);

    ensure_equals(toWKT(GEOSGetGeometryN(input_, 0)), "LINESTRING (0 0, 1 1)");
    ensure_equals(toWKT(GEOSGetGeometryN(input_, 1)), "CIRCULARSTRING (1 1, 2 0, 3 1)");
}

} // namespace tut

