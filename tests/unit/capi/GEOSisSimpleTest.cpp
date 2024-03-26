#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosissimple_data : public capitest::utility {};

typedef test_group<test_geosissimple_data> group;
typedef group::object object;

group test_geosissimple("capi::GEOSisSimple");

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");
    ensure_equals(1, GEOSisSimple(input_));
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 2, 1 2, 1 0)");
    ensure_equals(0, GEOSisSimple(input_));
}

template<>
template<>
void object::test<3>()
{
    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(input_ != nullptr);

    char ret = GEOSisSimple(input_);
    ensure_equals("error raised on curved geometry", ret, 2);
}

} // namespace tut
