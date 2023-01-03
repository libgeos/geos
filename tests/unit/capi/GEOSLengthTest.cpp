#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoslength_data : public capitest::utility {};

typedef test_group<test_geoslength_data> group;
typedef group::object object;

group test_geoslength("capi::GEOSLength");

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("LINESTRING (1 0, 5 0)");

    double length = -1;

    int ret = GEOSLength(input_, &length);

    ensure_equals(ret, 1);
    ensure_equals(length, 4);
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("POINT (0 0)");

    double length = -1;

    int ret = GEOSLength(input_, &length);

    ensure_equals(ret, 1);
    ensure_equals(length, 0);
}

template<>
template<>
void object::test<3>()
{
    input_ = GEOSGeomFromWKT("LINESTRING EMPTY");

    double length = -1;

    int ret = GEOSLength(input_, &length);

    ensure_equals(ret, 1);
    ensure_equals(length, 0);
}

} // namespace tut

