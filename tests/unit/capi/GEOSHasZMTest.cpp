#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoshaszm_data : public capitest::utility {};

typedef test_group<test_geoshaszm_data> group;
typedef group::object object;

group test_hasm("capi::GEOSHasZM");

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("POLYGON ((1 -2, 9 -2, 9 5, 1 5, 1 -2))");

    ensure_equals(GEOSHasZ(input_), 0);
    ensure_equals(GEOSHasM(input_), 0);
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("POINT M (1 2 3)");

    ensure_equals(GEOSHasZ(input_), 0);
    ensure_equals(GEOSHasM(input_), 1);
}

template<>
template<>
void object::test<3>()
{
    input_ = GEOSGeomFromWKT("POINT EMPTY");

    ensure_equals(GEOSHasZ(input_), 0);
    ensure_equals(GEOSHasM(input_), 0);
}

template<>
template<>
void object::test<4>()
{
    input_ = GEOSGeomFromWKT("POINT Z EMPTY");

    ensure_equals(GEOSHasZ(input_), 1);
    ensure_equals(GEOSHasM(input_), 0);
}

template<>
template<>
void object::test<5>()
{
    input_ = GEOSGeomFromWKT("POINT M EMPTY");

    ensure_equals(GEOSHasZ(input_), 0);
    ensure_equals(GEOSHasM(input_), 1);
}

template<>
template<>
void object::test<6>()
{
    input_ = GEOSGeomFromWKT("POINT ZM EMPTY");

    ensure_equals(GEOSHasZ(input_), 1);
    ensure_equals(GEOSHasM(input_), 1);
}

} // namespace tut

