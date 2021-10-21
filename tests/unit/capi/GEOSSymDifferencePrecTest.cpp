#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geossymdifferenceprec_data : public capitest::utility {};

typedef test_group<test_geossymdifferenceprec_data> group;
typedef group::object object;

group test_geossymdifferenceprec("capi::GEOSSymDifferencePrec");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING(50 100, 50 200)");
    ensure(geom1_);
    geom2_ = fromWKT("LINESTRING(50 50, 50 150)");
    ensure(geom2_);
    geom3_ = GEOSSymDifferencePrec(geom1_, geom2_, 15);

    ensure_equals("MULTILINESTRING ((45 150, 45 195), (45 45, 45 105))", toWKT(geom3_));
}

} // namespace tut

