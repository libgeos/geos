#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/constants.h>

#include "capi_test_utils.h"

namespace tut {
    //
    // Test Group
    //

    struct test_geosarea_data : public capitest::utility {};

    typedef test_group<test_geosarea_data> group;
    typedef group::object object;

    group test_geosarea("capi::GEOSArea");

    template<>
    template<>
    void object::test<1>()
    {
        set_test_name("curved inputs");

        input_ = fromWKT("CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING (0 0, 1 1, 2 0), (2 0, 0 0)))");
        ensure(input_ != nullptr);

        double area = -1;
        int ret = GEOSArea(input_, &area);
        ensure_equals(ret, 1);
        ensure_equals(area, geos::MATH_PI / 2);
    }

} // namespace tut
