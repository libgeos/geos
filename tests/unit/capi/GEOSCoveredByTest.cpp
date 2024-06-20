#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoscoveredby_data : public capitest::utility {};

typedef test_group<test_geoscoveredby_data> group;
typedef group::object object;

group test_geoscoveredby("capi::GEOSCoveredBy");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))");
    geom2_ = fromWKT("POLYGON ((5 5, 5 7, 7 7, 7 5, 5 5))");
    geom3_ = fromWKT("POLYGON ((20 20, 20 30, 30 30, 30 20, 20 20))");

    ensure_equals(1, GEOSCoveredBy(geom2_, geom1_));
    ensure_equals(0, GEOSCoveredBy(geom1_, geom2_));
    ensure_equals(0, GEOSCoveredBy(geom1_, geom3_));
    ensure_equals(0, GEOSCoveredBy(geom3_, geom1_));
    ensure_equals(0, GEOSCoveredBy(geom2_, geom3_));
    ensure_equals(0, GEOSCoveredBy(geom3_, geom2_));
}

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("LINESTRING (5 3, 5 4)");
    geom2_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 10 10, 20 0), (20 0, 0 0)))");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals("curved geometry not supported", GEOSCoveredBy(geom1_, geom2_), 2);
}

} // namespace tut

