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

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals("curved geometry not supported", GEOSRelatePattern(geom1_, geom2_, "0********"), 2);
    ensure_equals("curved geometry not supported", GEOSRelatePattern(geom2_, geom1_, "0********"), 2);
}

// invalid DE-9IM
template<>
template<>
void object::test<3>()
{
    geom1_ = fromWKT("POINT(1 2)");
    geom2_ = fromWKT("POINT(1 2)");

    ensure(geom1_);
    ensure(geom2_);

    // pattern too long
    ensure_equals(2, GEOSRelatePattern(geom1_, geom2_, "0FFFFF2120000000000000000000"));

    // pattern too short
    ensure_equals(2, GEOSRelatePattern(geom1_, geom2_, "0F"));

    // pattern has invalid characters
    ensure_equals(0, GEOSRelatePattern(geom1_, geom2_, "123456789"));
}

} // namespace tut

