#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosissimple_data : public capitest::utility {};

typedef test_group<test_geosissimple_data> group;
typedef group::object object;

group test_geosissimple("capi::GEOSisSimple");

template<>
template<>
void object::test<1>()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");
    ensure_equals(1, GEOSisSimple(input_));
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 2, 1 2, 1 0)");
    ensure_equals(0, GEOSisSimple(input_));
}

template<>
template<>
void object::test<3>()
{
    set_test_name("curved inputs");
    useContext();

    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(input_ != nullptr);

    ensure_equals(GEOSisSimple_r(ctxt_, input_), 2);
    useCurveConversion();
    ensure_equals(GEOSisSimple_r(ctxt_, input_), 1);
}

} // namespace tut
