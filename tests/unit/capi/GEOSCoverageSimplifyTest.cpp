//
// Test Suite for C-API GEOSCoverageSimplifyVW

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capicoveragesimplify_data : public capitest::utility {

    test_capicoveragesimplify_data() {
    }

    ~test_capicoveragesimplify_data() {
    }

};


typedef test_group<test_capicoveragesimplify_data> group;
typedef group::object object;

group test_capicoveragesimplify_group("capi::GEOSCoverageSimplify");

//
// Test Cases
//

// GEOSCoverageSimplifyVW - all
template<>
template<> void object::test<1>
()
{
    const char* inputWKT = "GEOMETRYCOLLECTION(POLYGON ((100 100, 200 200, 300 100, 200 101, 100 100)), POLYGON ((150 0, 100 100, 200 101, 300 100, 250 0, 150 0)))";

    input_ = fromWKT(inputWKT);
    result_ = GEOSCoverageSimplifyVW(input_, 10.0, 0);

    ensure( result_ != nullptr );
    ensure( GEOSGeomTypeId(result_) == GEOS_GEOMETRYCOLLECTION );

    const char* expectedWKT = "GEOMETRYCOLLECTION(POLYGON ((100 100, 200 200, 300 100, 100 100)), POLYGON ((150 0, 100 100, 300 100, 250 0, 150 0)))";

    expected_ = fromWKT(expectedWKT);

    // std::cout << toWKT(result_) << std::endl;
    // std::cout << toWKT(expected_) << std::endl;

    ensure_geometry_equals(result_, expected_, 0.1);
}

// GEOSCoverageSimplifyVW - inner
template<>
template<> void object::test<2>
()
{
    const char* inputWKT = "GEOMETRYCOLLECTION( POLYGON ((1 9, 5 9.1, 9 9, 9 5, 5 5.1, 1 5, 1 9)), POLYGON ((1 1, 1 5, 5 5.1, 9 5, 9 1, 1 1)))";

    input_ = fromWKT(inputWKT);
    result_ = GEOSCoverageSimplifyVW(input_, 1.0, 1);

    ensure( result_ != nullptr );
    ensure( GEOSGeomTypeId(result_) == GEOS_GEOMETRYCOLLECTION );

    const char* expectedWKT = "GEOMETRYCOLLECTION(POLYGON ((9 5, 1 5, 1 9, 5 9.1, 9 9, 9 5)), POLYGON ((9 5, 1 5, 1 1, 9 1, 9 5)))";

    expected_ = fromWKT(expectedWKT);
    ensure_geometry_equals(result_, expected_, 0.1);
}


// GEOSCoverageSimplifyVW
template<>
template<> void object::test<3>
()
{
    const char* inputWKT = "GEOMETRYCOLLECTION(POLYGON(( 0 0,10 0,10.1 5,10 10,0 10,0 0)),POLYGON((10 0,20 0,20 10,10 10,10.1 5,10 0)))";

    input_ = fromWKT(inputWKT);
    result_ = GEOSCoverageSimplifyVW(input_, 1.0, 0);

    ensure( result_ != nullptr );
    ensure( GEOSGeomTypeId(result_) == GEOS_GEOMETRYCOLLECTION );

    const char* expectedWKT = "GEOMETRYCOLLECTION(POLYGON((0 0,10 0,10 10,0 10,0 0)),POLYGON((10 0,20 0,20 10,10 10,10 0)))";

    expected_ = fromWKT(expectedWKT);
    ensure_geometry_equals(result_, expected_, 0.1);
}

template<>
template<>
void object::test<4>
()
{
    set_test_name("curved inputs");
    useContext();

    input_ = fromWKT("GEOMETRYCOLLECTION ( "
        "CURVEPOLYGON (COMPOUNDCURVE ( (4 0, 0 0, 0 4, 4 4), CIRCULARSTRING (4 4, 3.95 2, 4 0))), "
        "CURVEPOLYGON (CIRCULARSTRING (4 4, 6 2, 4 0, 3.95 2, 4 4)))");

    result_ = GEOSCoverageSimplifyVW_r(ctxt_, input_, 1, false);
    ensure("curved geometry not supported", result_ == nullptr);

    useCurveConversion();

    result_ = GEOSCoverageSimplifyVW_r(ctxt_, input_, 0.2, false);
    ensure(result_);

    ensure_equals(GEOSGetNumGeometries_r(ctxt_, result_), 2);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 0)), GEOS_POLYGON);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, GEOSGetGeometryN_r(ctxt_, result_, 1)), GEOS_CURVEPOLYGON);
}


} // namespace tut
