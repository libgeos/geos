#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeossplit_data : public capitest::utility {
};

typedef test_group<test_capigeossplit_data> group;
typedef group::object object;

group test_capigeossplit_group("capi::GEOSSplit");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    geom2_ = fromWKT("LINESTRING (-5 5, 5 5, 5 -5)");

    result_ = GEOSSplit(geom1_, geom2_);
    ensure(result_);

    expected_ = fromWKT("GEOMETRYCOLLECTION (POLYGON ((0 0, 5 0, 5 5, 0 5, 0 0)), POLYGON ((0 5, 5 5, 5 0, 10 0, 10 10, 0 10, 0 5)))");

    ensure_geometry_equals(result_, expected_);
}

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("POINT (3 7)");
    geom2_ = fromWKT("LINESTRING (3 7, 3 8)");
    geom3_ = fromWKT("POINT (3 7)");

    result_ = GEOSSplit(geom1_, geom2_); // cannot split a point geometry
    ensure(!result_);

    result_ = GEOSSplit(geom1_, geom3_); // cannot split a point geometry
    ensure(!result_);
}

template<>
template<>
void object::test<3>()
{
    geom1_ = fromWKT("GEOMETRYCOLLECTION (POINT (3 7), LINESTRING (3 7, 3 8))");
    geom2_ = fromWKT("LINESTRING (3 7, 3 8)");
    geom3_ = fromWKT("POINT (3 7)");

    result_ = GEOSSplit(geom1_, geom2_); // cannot split collection with a point geometry
    ensure(!result_);

    result_ = GEOSSplit(geom1_, geom3_); // cannot split collection with a point geometry
    ensure(!result_);
}

} // namespace tut

