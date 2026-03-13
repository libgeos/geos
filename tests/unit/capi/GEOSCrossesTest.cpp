#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoscrosses_data : public capitest::utility {};

typedef test_group<test_geoscrosses_data> group;
typedef group::object object;

group test_geoscrosses("capi::GEOSCrosses");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING (1 1, 10 10)");
    geom2_ = fromWKT("LINESTRING (10 1, 1 10)");
    geom3_ = fromWKT("LINESTRING (20 20, 30 30)");

    ensure_equals(1, GEOSCrosses(geom1_, geom2_));
    ensure_equals(1, GEOSCrosses(geom2_, geom1_));
    ensure_equals(0, GEOSCrosses(geom1_, geom3_));
    ensure_equals(0, GEOSCrosses(geom3_, geom1_));
    ensure_equals(0, GEOSCrosses(geom2_, geom3_));
    ensure_equals(0, GEOSCrosses(geom3_, geom2_));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("GEOSCrosses with automatic linearization");
    useContext();

    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals(GEOSCrosses_r(ctxt_, geom1_, geom2_), 2);
    ensure_equals(GEOSCrosses_r(ctxt_, geom2_, geom1_), 2);

    useCurveConversion();

    ensure_equals(GEOSCrosses_r(ctxt_, geom1_, geom2_), 1);
    ensure_equals(GEOSCrosses_r(ctxt_, geom2_, geom1_), 1);
}

} // namespace tut

