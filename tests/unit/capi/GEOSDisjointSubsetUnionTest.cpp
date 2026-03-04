#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capidisjointsubsetunion_data : public capitest::utility {};


typedef test_group<test_capidisjointsubsetunion_data> group;
typedef group::object object;

group test_capidisjointsubsetunion_group("capi::GEOSDisjointSubsetUnion");

//
// Test Cases
//


template<>
template<>
void object::test<1>()
{
    input_ = fromWKT("POLYGON EMPTY");
    GEOSSetSRID(input_, 1234);

    result_ = GEOSDisjointSubsetUnion(input_);

    ensure(GEOSisEmpty(result_));
    ensure_equals(GEOSGetSRID(input_), GEOSGetSRID(result_));
}

template<>
template<>
void object::test<2>()
{
    input_ = fromWKT("MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((1 0, 2 0, 2 1, 1 1, 1 0)), ((3 3, 4 3, 4 4, 3 3)))");
    expected_ = fromWKT("MULTIPOLYGON (((0 0, 1 0, 2 0, 2 1, 1 1, 0 1, 0 0)), ((3 3, 4 3, 4 4, 3 3)))");
    result_ = GEOSDisjointSubsetUnion(input_);
    ensure_geometry_equals(result_, expected_);
}

template <>
template <>
void object::test<3>()
{
    set_test_name("curved inputs");
    useContext();

    input_ = fromWKT("MULTISURFACE ("
                     "CURVEPOLYGON (COMPOUNDCURVE(CIRCULARSTRING (0 0, 5 5, 10 0), (10 0, 10 -10, 0 -10, 0 0))),"
                     "((10 -10, 20 -10, 20 0, 10 0, 10 -10)),"
                     "CURVEPOLYGON (CIRCULARSTRING (100 100, 110 110, 120 100, 110 90, 100 100)))");
    ensure(input_);

    result_ = GEOSDisjointSubsetUnion_r(ctxt_, input_);
    ensure("curved geometry not supported", result_ == nullptr);

    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);

    // Input converted to line, output not converted to curve
    result_ = GEOSDisjointSubsetUnion_r(ctxt_, input_);
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_MULTIPOLYGON);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 2);
    GEOSGeom_destroy_r(ctxt_, result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSDisjointSubsetUnion_r(ctxt_, input_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_MULTISURFACE);
    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_CURVEPOLYGON);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 1)), GEOS_CURVEPOLYGON);
}

} // namespace tut

