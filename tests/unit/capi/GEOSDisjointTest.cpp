#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosdisjoint_data : public capitest::utility {};

typedef test_group<test_geosdisjoint_data> group;
typedef group::object object;

group test_geosdisjoint("capi::GEOSDisjoint");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POINT (0 0)");
    geom2_ = fromWKT("LINESTRING(2 0, 0 2)");
    geom3_ = fromWKT("LINESTRING(0 0, 0 2)");

    ensure_equals(1, GEOSDisjoint(geom1_, geom2_));
    ensure_equals(0, GEOSDisjoint(geom1_, geom3_));
}

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals("curved geometry not supported", GEOSDisjoint(geom1_, geom2_), 2);
    ensure_equals("curved geometry not supported", GEOSDisjoint(geom2_, geom1_), 2);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("GEOSDisjoint with automatic linearization");

    useContext();

    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("CIRCULARSTRING (2 0, 3 -1, 4 0)");

    ensure_equals(2, GEOSDisjoint_r(ctxt_, geom1_, geom2_));
    useCurveConversion();
    ensure_equals(0, GEOSDisjoint_r(ctxt_, geom1_, geom2_));
}

template<>
template<>
void object::test<4>()
{
    set_test_name("MultiSurface / MultiPoint PIP");

    geom1_ = fromWKT("MULTISURFACE(POLYGON ((100 100, 200 100, 200 200, 100 100)), CURVEPOLYGON (COMPOUNDCURVE(CIRCULARSTRING(0 0, 1 1, 2 0), (2 0, 0 0))))");
    geom2_ = fromWKT("MULTIPOINT ((5000 5000), (0.1556955 0.5355459))");

    ensure_equals(GEOSDisjoint(geom1_, geom2_), 0);
    ensure_equals(GEOSDisjoint(geom2_, geom1_), 0);
}


template<>
template<>
void object::test<5>()
{
    set_test_name("Empty MultiPoint is disjoint from MultiSurface");

    geom1_ = fromWKT("MULTISURFACE(CURVEPOLYGON (COMPOUNDCURVE(CIRCULARSTRING(0 0, 1 1, 2 0), (2 0, 0 0))))");
    geom2_ = fromWKT("MULTIPOINT EMPTY");

    ensure_equals(GEOSDisjoint(geom1_, geom2_), 1);
    ensure_equals(GEOSDisjoint(geom2_, geom1_), 1);
}

} // namespace tut

