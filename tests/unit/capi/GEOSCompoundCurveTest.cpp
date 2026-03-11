// Test Suite for C-API CompoundCurve functions

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
struct test_capicompoundcurve_data : public capitest::utility {

    GEOSGeometry* cc_;

    test_capicompoundcurve_data() {
        cc_ = fromWKT("COMPOUNDCURVE(CIRCULARSTRING ZM (-5 0 6 7, 4 3 7 8 , 0 -5 8 9), (0 -5 8 9, -5 -5 9 10))");
    }

    ~test_capicompoundcurve_data() override {
        GEOSGeom_destroy(cc_);
    }
};

typedef test_group<test_capicompoundcurve_data> group;
typedef group::object object;

group test_capicompoundcurve_group("capi::GEOSCompoundCurve");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    set_test_name("GEOSisClosed");

    ensure_equals(GEOSisClosed(cc_), 0);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("GEOSGeomGetPointN");

    double x, y, z, m;
    result_ = GEOSGeomGetPointN(cc_, 1);
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
    result_ = GEOSGeomGetStartPoint(cc_);
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
    result_ = GEOSGeomGetEndPoint(cc_);
    GEOSGeomGetX(result_, &x);
    GEOSGeomGetY(result_, &y);
    GEOSGeomGetZ(result_, &z);
    GEOSGeomGetM(result_, &m);

    ensure_equals(x, -5);
    ensure_equals(y, -5);
    ensure_equals(z, 9);
    ensure_equals(m, 10);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("GEOSGeomGetNumPoints");

    ensure_equals(GEOSGeomGetNumPoints(cc_), 5);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("GEOSGeom_getCoordSeq");

    const GEOSCoordSequence* seq = GEOSGeom_getCoordSeq(cc_);
    ensure(!seq);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("GEOSGeom_getDimensions");

    ensure_equals(GEOSGeom_getDimensions(cc_), 1);
}

} // namespace tut
