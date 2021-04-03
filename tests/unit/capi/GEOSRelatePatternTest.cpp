#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosrelatepattern_data : public capitest::utility {};

typedef test_group<test_geosrelatepattern_data> group;
typedef group::object object;

group test_geosrelatepattern("capi::GEOSRelatePattern");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POINT(1 2)");
    ensure(nullptr != geom1_);
    geom2_ = fromWKT("POINT(1 2)");
    ensure(nullptr != geom2_);
    geom3_ =  GEOSBuffer(geom2_, 2, 8);
    ensure(nullptr != geom3_);

    ensure_equals(1, GEOSRelatePattern(geom1_, geom3_, "0FFFFF212"));
    ensure_equals(1, GEOSRelatePattern(geom1_, geom3_, "*FF*FF212"));
}

} // namespace tut

