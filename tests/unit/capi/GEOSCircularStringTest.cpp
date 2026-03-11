// Test Suite for C-API CircularString functions

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cmath>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capicircularstring_data : public capitest::utility {

    GEOSGeometry* cs_;

    test_capicircularstring_data() {
        cs_ = fromWKT("CIRCULARSTRING ZM (-5 0 6 7, 4 3 7 8 , 0 -5 8 9)");
    }

    ~test_capicircularstring_data() override {
        GEOSGeom_destroy(cs_);
    }
};

typedef test_group<test_capicircularstring_data> group;
typedef group::object object;

group test_capicircularstring_group("capi::GEOSCircularString");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    set_test_name("GEOSisClosed");

    ensure_equals(GEOSisClosed(cs_), 0);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("GEOSGeomGetPointN");

    double x, y, z, m;
    result_ = GEOSGeomGetPointN(cs_, 1);
    GEOSGeomGetX(result_, &x);
    GEOSGeomGetY(result_, &y);
    GEOSGeomGetZ(result_, &z);
    GEOSGeomGetM(result_, &m);

    ensure_equals(x, 4);
    ensure_equals(y, 3);
    ensure_equals(z, 7);
    ensure_equals(m, 8);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("GEOSGeomGetStartPoint");

    double x, y, z, m;
    result_ = GEOSGeomGetStartPoint(cs_);
    GEOSGeomGetX(result_, &x);
    GEOSGeomGetY(result_, &y);
    GEOSGeomGetZ(result_, &z);
    GEOSGeomGetM(result_, &m);

    ensure_equals(x, -5);
    ensure_equals(y, 0);
    ensure_equals(z, 6);
    ensure_equals(m, 7);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("GEOSGeomGetEndPoint");

    double x, y, z, m;
    result_ = GEOSGeomGetEndPoint(cs_);
    GEOSGeomGetX(result_, &x);
    GEOSGeomGetY(result_, &y);
    GEOSGeomGetZ(result_, &z);
    GEOSGeomGetM(result_, &m);

    ensure_equals(x, 0);
    ensure_equals(y, -5);
    ensure_equals(z, 8);
    ensure_equals(m, 9);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("GEOSGeomGetNumPoints");

    ensure_equals(GEOSGeomGetNumPoints(cs_), 3);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("GEOSGeom_getCoordSeq");

    const GEOSCoordSequence* seq = GEOSGeom_getCoordSeq(cs_);
    ensure(seq);

    unsigned int size = 0;
    ensure(GEOSCoordSeq_getSize(seq, &size));
    ensure_equals(size, 3u);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("GEOSGeom_getDimensions");

    ensure_equals(GEOSGeom_getDimensions(cs_), 1);
}

} // namespace tut
