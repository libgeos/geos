#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geossymdifference_data : public capitest::utility {};

typedef test_group<test_geossymdifference_data> group;
typedef group::object object;

group test_geossymdifference("capi::GEOSSymDifference");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING(50 100, 50 200)");
    ensure(geom1_);
    geom2_ = fromWKT("LINESTRING(50 50, 50 150)");
    ensure(geom2_);
    geom3_ = GEOSSymDifference(geom1_, geom2_);

    ensure_equals("MULTILINESTRING ((50 150, 50 200), (50 50, 50 100))", toWKT(geom3_));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("curved inputs, curved output");
    useContext();

    geom1_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE( CIRCULARSTRING (-5 0, 0 5, 5 0), (5 0, -5 0)))");
    geom2_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE( CIRCULARSTRING (-5 5, 0 0, 5 5), (5 5, -5 5)))");

    ensure(geom1_);
    ensure(geom2_);
    GEOSSetSRID_r(ctxt_, geom1_, 4326);

    result_ = GEOSSymDifference_r(ctxt_, geom1_, geom2_);
    ensure("curved geometry not supported", result_ == nullptr);

    GEOSCurveToLineParams_setTolerance_r(ctxt_, curveToLineParams_, GEOS_CURVETOLINE_STEP_DEGREES, 1);
    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);

    // Input converted to line, output not converted to curve
    result_ = GEOSSymDifference_r(ctxt_, geom1_, geom2_);
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_MULTIPOLYGON);
    GEOSGeom_destroy_r(ctxt_, result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSSymDifference_r(ctxt_, geom1_, geom2_);

    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_MULTISURFACE);
    ensure_equals(GEOSGetSRID_r(ctxt_, result_), 4326);
}

} // namespace tut

