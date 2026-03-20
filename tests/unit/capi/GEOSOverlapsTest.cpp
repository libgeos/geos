#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosoverlaps_data : public capitest::utility {};

typedef test_group<test_geosoverlaps_data> group;
typedef group::object object;

group test_geosoverlaps("capi::GEOSOverlaps");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))");
    ensure(nullptr != geom1_);
    geom2_ = fromWKT("POLYGON ((5 5, 5 15, 15 15, 15 5, 5 5))");
    ensure(nullptr != geom2_);
    geom3_ = fromWKT("POLYGON ((20 20, 20 30, 30 30, 30 20, 20 20))");
    ensure(nullptr != geom3_);

    ensure_equals(1, GEOSOverlaps(geom1_, geom2_));
    ensure_equals(1, GEOSOverlaps(geom2_, geom1_));
    ensure_equals(0, GEOSOverlaps(geom1_, geom3_));
    ensure_equals(0, GEOSOverlaps(geom3_, geom1_));
    ensure_equals(0, GEOSOverlaps(geom2_, geom3_));
    ensure_equals(0, GEOSOverlaps(geom3_, geom2_));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("GEOSOverlaps with automatic linearization");
    useContext();

    geom1_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 1 1, 2 0), (2 0, 0 0)))");
    geom2_ = fromWKT("POLYGON ((1 0, 2 0, 2 1, 1 0 ))");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals(GEOSOverlaps_r(ctxt_, geom1_, geom2_), 2);
    ensure_equals(GEOSOverlaps_r(ctxt_, geom2_, geom1_), 2);
    useCurveConversion();
    ensure_equals(GEOSOverlaps_r(ctxt_, geom1_, geom2_), 1);
    ensure_equals(GEOSOverlaps_r(ctxt_, geom2_, geom1_), 1);
}

} // namespace tut

