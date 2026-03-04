#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoscoverageclean_data : public capitest::utility
{
    GEOSCoverageCleanParams* ccp_ = nullptr;

    ~test_geoscoverageclean_data() override {
        if (ctxt_) {
            GEOSCoverageCleanParams_destroy_r(ctxt_, ccp_);
        } else {
            GEOSCoverageCleanParams_destroy(ccp_);
        }
    }
};

typedef test_group<test_geoscoverageclean_data> group;
typedef group::object object;

group test_geoscoverageclean("capi::GEOSCoverageClean");

template<>
template<>
void object::test<1>()
{
    set_test_name("curved inputs");
    useContext();

    input_ = fromWKT("GEOMETRYCOLLECTION ("
                     "POLYGON ((0 0, 10 0, 10.0001 5, 10 10, 0 10, 0 0)),"
                     "CURVEPOLYGON (COMPOUNDCURVE ((20 0, 10 0, 10 10, 20 10), CIRCULARSTRING (20 10, 25 5, 20 0))))");

    result_ = GEOSCoverageClean_r(ctxt_, input_);
    ensure(!result_);

    ccp_ = GEOSCoverageCleanParams_create_r(ctxt_);
    result_ = GEOSCoverageCleanWithParams_r(ctxt_, input_, ccp_);
    ensure(!result_);

    useCurveConversion();

    result_ = GEOSCoverageClean_r(ctxt_, input_);
    ensure(result_);

    expected_ = fromWKT("GEOMETRYCOLLECTION ("
                     "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0)),"
                     "CURVEPOLYGON (COMPOUNDCURVE ((20 0, 10 0, 10 10, 20 10), CIRCULARSTRING (20 10, 25 5, 20 0))))");

    ensure_geometry_equals(result_, expected_);
}
} // namespace tut

