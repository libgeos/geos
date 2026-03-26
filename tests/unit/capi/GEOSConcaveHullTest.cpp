//
// Test Suite for C-API GEOSConcaveHull

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
struct test_capigeosconcavehull_data : public capitest::utility {
};

typedef test_group<test_capigeosconcavehull_data> group;
typedef group::object object;

group test_capigeosconcavehull_group("capi::GEOSConcaveHull");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    input_ = fromWKT("MULTIPOINT ((10 90), (10 10), (90 10), (90 90), (40 40), (60 30), (30 70), (40 60), (60 50), (60 72), (47 66), (90 60))");
    expected_ = fromWKT("POLYGON ((30 70, 10 90, 60 72, 90 90, 90 60, 90 10, 60 30, 10 10, 40 40, 60 50, 47 66, 40 60, 30 70))");
    result_ = GEOSConcaveHull(input_, 0, 0);
    ensure(nullptr != result_);
    ensure(0 == GEOSisEmpty(result_));
    ensure_geometry_equals(result_, expected_);
}

template<>
template<>
void object::test<2>()
{
    input_ = fromWKT("MULTIPOINT ((10 90), (10 10), (90 10), (90 90), (40 40), (60 30), (30 70), (40 60), (60 50), (60 72), (47 66), (90 60))");
    expected_ = fromWKT("POLYGON ((30 70, 10 90, 60 72, 90 90, 90 60, 90 10, 60 30, 10 10, 40 40, 30 70))");
    result_ = GEOSConcaveHullByLength(input_, 50, 0);
    ensure(nullptr != result_);
    ensure(0 == GEOSisEmpty(result_));
    ensure_geometry_equals(result_, expected_);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("curved inputs");
    useContext();

    input_ = fromWKT("CURVEPOLYGON (CIRCULARSTRING (0 0, 1 1, 2 0, 1 0.8, 0 0))");
    ensure(input_ != nullptr);

    result_ = GEOSConcaveHull_r(ctxt_, input_, 0.5, 0);
    ensure(result_ == nullptr);

    result_ = GEOSConcaveHullByLength_r(ctxt_, input_, 10, 0);
    ensure(result_ == nullptr);

    useCurveConversion();

    result_ = GEOSConcaveHull_r(ctxt_, input_, 0.5, 0);
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_POLYGON);
    GEOSGeom_destroy_r(ctxt_, result_);

    result_ = GEOSConcaveHullByLength_r(ctxt_, input_, 10, 0);
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_POLYGON);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("MULTIPOINT ZM");

    input_ = fromWKT("MULTIPOINT ZM (0 0 1 2, 1 0 3 4, 1 1 5 6, 1 8 11 4, 0.5 0.5 -4 -7)");
    result_ = GEOSConcaveHull(input_, 0, 0);

    ensure(result_);
    ensure(GEOSHasZ(result_));
    ensure(!GEOSHasM(result_));
}


} // namespace tut
