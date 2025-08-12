#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosissimpledetail_data : public capitest::utility {};

typedef test_group<test_geosissimpledetail_data> group;
typedef group::object object;

group test_geosissimpledetail("capi::GEOSisSimpleDetail");

template<>
template<>
void object::test<1>()
{
    set_test_name("simple LineString");

    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");
    ensure_equals(1, GEOSisSimpleDetail(input_, 0, &result_));
    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("LineString with single self-intersection");

    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 2, 1 2, 1 0)");
    ensure_equals(0, GEOSisSimpleDetail(input_, 0, &result_));
    ensure_geometry_equals(result_, "POINT (1 1)");
}

template<>
template<>
void object::test<3>()
{
    set_test_name("LineString with multiple self-intersections");

    input_ = GEOSGeomFromWKT("LINESTRING (2 1, 0 0, 2 2, 1 2, 1 0)");
    expected_ = GEOSGeomFromWKT("MULTIPOINT (1 1, 1 0.5)");

    ensure_equals(0, GEOSisSimpleDetail(input_, 0, &result_));
    ensure(result_ != nullptr);
    ensure(GEOSEquals(result_, GEOSGetGeometryN(expected_, 0)) || GEOSEquals(result_, GEOSGetGeometryN(expected_, 1)));
    GEOSGeom_destroy(result_);

    ensure_equals(0, GEOSisSimpleDetail(input_, 1, &result_));
    ensure(result_ != nullptr);
    ensure_geometry_equals(result_, expected_);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("error raised on curved geometry");

    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(input_ != nullptr);

    ensure_equals(GEOSisSimpleDetail(input_, 0, &result_), 2);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("points are always simple");

    input_ = GEOSGeomFromWKT("POINT (3 2)");
    ensure_equals(1, GEOSisSimpleDetail(input_, 0, &result_));
    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("empty LineString is simple");

    input_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    ensure_equals(1, GEOSisSimpleDetail(input_, 0, &result_));
    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("Polygon is non-simple if ring is non-simple");

    input_ = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 0 1, 1 1, 0 0))");
    ensure_equals(0, GEOSisSimpleDetail(input_, 0, &result_));
    ensure_geometry_equals(result_, "POINT (0.5 0.5)");
}

} // namespace tut
