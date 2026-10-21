#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoscovers_data : public capitest::utility {};

typedef test_group<test_geoscovers_data> group;
typedef group::object object;

group test_geoscovers("capi::GEOSCovers");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))");
    geom2_ = fromWKT("POLYGON ((5 5, 5 7, 7 7, 7 5, 5 5))");
    geom3_ = fromWKT("POLYGON ((20 20, 20 30, 30 30, 30 20, 20 20))");

    ensure_equals(1, GEOSCovers(geom1_, geom2_));
    ensure_equals(0, GEOSCovers(geom2_, geom1_));
    ensure_equals(0, GEOSCovers(geom1_, geom3_));
    ensure_equals(0, GEOSCovers(geom3_, geom1_));
    ensure_equals(0, GEOSCovers(geom2_, geom3_));
    ensure_equals(0, GEOSCovers(geom3_, geom2_));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("GEOSCovers with automatic linearization");
    useContext();

    geom1_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 1 1, 2 0), (2 0, 0 0)))");
    geom2_ = fromWKT("LINESTRING (1 0.5, 1 0.6)");

    ensure(geom1_);
    ensure(geom2_);

    ensure_equals(GEOSCovers_r(ctxt_, geom1_, geom2_), 2);
    useCurveConversion();
    ensure_equals(GEOSCovers_r(ctxt_, geom1_, geom2_), 1);
    ensure_equals(GEOSCovers_r(ctxt_, geom2_, geom1_), 0);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("line covers collinear point");

    geom1_ = fromWKT("LINESTRING (1 0, 0 2)");
    geom2_ = fromWKT("POINT (0.9 0.2)");
    geom3_ = fromWKT("POINT (0.1 1.8)");

    ensure_equals("0.9 0.2", 1, GEOSCovers(geom1_, geom2_));
    ensure_equals("0.1 1.8", 1, GEOSCovers(geom1_, geom3_));
}

template<>
template<>
void object::test<4>()
{
    set_test_name("line does not cover point off a long segment");

    geom1_ = fromWKT("LINESTRING (0 0, 10000000000000000 10000000000000000)");
    geom2_ = fromWKT("POINT (5000000000000000 5000000000000001)");

    ensure_equals(0, GEOSCovers(geom1_, geom2_));
}

} // namespace tut

