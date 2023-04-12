#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeomgetxyzm_data : public capitest::utility {};

typedef test_group<test_geosgeomgetxyzm_data> group;
typedef group::object object;

group test_geosgeomgetxyzm("capi::GEOSGeomGetXYZM");

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("POINT (1 2)");
    double x = 0;
    double y = 0;
    double z = 0;
    double m = 0;

    ensure_equals(GEOSGeomGetX(input_, &x), 1);
    ensure_equals(GEOSGeomGetY(input_, &y), 1);
    ensure_equals(GEOSGeomGetZ(input_, &z), 1);
    ensure_equals(GEOSGeomGetM(input_, &m), 1);

    ensure_equals(x, 1);
    ensure_equals(y, 2);
    ensure(std::isnan(z));
    ensure(std::isnan(m));
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("POINT Z (1 2 3)");
    double z = 0;
    double m = 0;

    ensure_equals(GEOSGeomGetZ(input_, &z), 1);
    ensure_equals(GEOSGeomGetM(input_, &m), 1);

    ensure_equals(z, 3);
    ensure(std::isnan(m));
}

template<>
template<>
void object::test<3>()
{
    input_ = GEOSGeomFromWKT("POINT M (1 2 4)");
    double z = 0;
    double m = 0;

    ensure_equals(GEOSGeomGetZ(input_, &z), 1);
    ensure_equals(GEOSGeomGetM(input_, &m), 1);

    ensure(std::isnan(z));
    ensure_equals(m, 4);
}

template<>
template<>
void object::test<4>()
{
    input_ = GEOSGeomFromWKT("POINT ZM (1 2 3 4)");
    double z = 0;
    double m = 0;

    ensure_equals(GEOSGeomGetZ(input_, &z), 1);
    ensure_equals(GEOSGeomGetM(input_, &m), 1);

    ensure_equals(z, 3);
    ensure_equals(m, 4);
}

template<>
template<>
void object::test<5>()
{
    input_ = GEOSGeomFromWKT("POINT EMPTY");

    double x = 0;
    double y = 0;
    double z = 0;
    double m = 0;

    ensure_equals(GEOSGeomGetX(input_, &x), 0);
    ensure_equals(GEOSGeomGetY(input_, &y), 0);
    ensure_equals(GEOSGeomGetZ(input_, &z), 0);
    ensure_equals(GEOSGeomGetM(input_, &m), 0);
}

template<>
template<>
void object::test<6>()
{
    input_ = GEOSGeomFromWKT("LINESTRING (1 1, 2 2)");

    double x = 0;
    double y = 0;
    double z = 0;
    double m = 0;

    ensure_equals(GEOSGeomGetX(input_, &x), 0);
    ensure_equals(GEOSGeomGetY(input_, &y), 0);
    ensure_equals(GEOSGeomGetZ(input_, &z), 0);
    ensure_equals(GEOSGeomGetM(input_, &m), 0);
}


} // namespace tut

