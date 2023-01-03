#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeomgetxyz_data : public capitest::utility {};

typedef test_group<test_geosgeomgetxyz_data> group;
typedef group::object object;

group test_geosgeomgetxyz("capi::GEOSGeomGetXYZ");

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("POINT (1 2)");
    double x = 0;
    double y = 0;
    double z = 0;

    ensure_equals(GEOSGeomGetX(input_, &x), 1);
    ensure_equals(GEOSGeomGetY(input_, &y), 1);
    ensure_equals(GEOSGeomGetZ(input_, &z), 1);

    ensure_equals(x, 1);
    ensure_equals(y, 2);
    ensure(std::isnan(z));
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("POINT (1 2 3)");
    double z = 0;

    ensure_equals(GEOSGeomGetZ(input_, &z), 1);
    ensure_equals(z, 3);
}

template<>
template<>
void object::test<3>()
{
    input_ = GEOSGeomFromWKT("POINT EMPTY");

    double x = 0;
    double y = 0;
    double z = 0;

    ensure_equals(GEOSGeomGetX(input_, &x), 0);
    ensure_equals(GEOSGeomGetY(input_, &y), 0);
    ensure_equals(GEOSGeomGetZ(input_, &z), 0);
}

template<>
template<>
void object::test<4>()
{
    input_ = GEOSGeomFromWKT("LINESTRING (1 1, 2 2)");

    double x = 0;
    double y = 0;

    ensure_equals(GEOSGeomGetX(input_, &x), 0);
    ensure_equals(GEOSGeomGetY(input_, &y), 0);
}


} // namespace tut

