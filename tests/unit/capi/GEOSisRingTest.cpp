//
// Test Suite for C-API GEOSisClosed

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capiisring_data : public capitest::utility {};

typedef test_group<test_capiisring_data> group;
typedef group::object object;

group test_capiisring_group("capi::GEOSisRing");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 1 0, 1 1)");
    char r = GEOSisRing(geom1_);
    ensure_equals(r, 0);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 0, 1 1, 0 0)");
    char r = GEOSisRing(geom1_);
    ensure_equals(r, 1);
}

template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POINT (1 1)");
    char r = GEOSisRing(geom1_);
    ensure_equals(r, 0);
}

template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING ((0 0, 1 0, 1 1, 0 0))");
    char r = GEOSisRing(geom1_);
    ensure_equals(r, 0);
}

template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    char r = GEOSisRing(geom1_);
    ensure_equals(r, 0);
}


template<>
template<>
void object::test<6>
()
{
    set_test_name("curved inputs");
    useContext();

    geom1_ = fromWKT("COMPOUNDCURVE (CIRCULARSTRING (0 0, 1 1, 2 0), (2 0, 0 0))");
    ensure(geom1_);

    //ensure_equals(GEOSisRing_r(ctxt_, geom1_), 2);
    useCurveConversion();
    ensure_equals(GEOSisRing_r(ctxt_, geom1_), 1);
}

} // namespace tut
