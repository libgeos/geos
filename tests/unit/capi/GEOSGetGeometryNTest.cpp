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

    GEOSGeometry* result = const_cast<GEOSGeometry*>(GEOSGetGeometryN(geom1_, 0));
    ensure(nullptr != result);
    ensure_equals("POINT (1 1)", toWKT(result));
    
    result = const_cast<GEOSGeometry*>(GEOSGetGeometryN(geom1_, 2));
    ensure(nullptr != result);
    ensure_equals("POINT (3 3)", toWKT(result));

    ensure(GEOSGetGeometryN(geom1_, -1) == nullptr);
}

} // namespace tut

