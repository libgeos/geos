#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geostouches_data : public capitest::utility {};

typedef test_group<test_geostouches_data> group;
typedef group::object object;

group test_geostouches("capi::GEOSTouches");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING (1 1, 10 1)");
    ensure(nullptr != geom1_);
    geom2_ = fromWKT("LINESTRING (5 1, 5 10)");
    ensure(nullptr != geom2_);
    geom3_ = fromWKT("LINESTRING (20 20, 30 30)");
    ensure(nullptr != geom3_);

    ensure_equals(1, GEOSTouches(geom1_, geom2_));
    ensure_equals(1, GEOSTouches(geom2_, geom1_));
    ensure_equals(0, GEOSTouches(geom1_, geom3_));
    ensure_equals(0, GEOSTouches(geom3_, geom1_));
    ensure_equals(0, GEOSTouches(geom2_, geom3_));
    ensure_equals(0, GEOSTouches(geom3_, geom2_));
}

} // namespace tut

