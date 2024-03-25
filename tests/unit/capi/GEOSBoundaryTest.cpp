#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosboundary_data : public capitest::utility {};

typedef test_group<test_geosboundary_data> group;
typedef group::object object;

group test_geosboundary("capi::GEOSBoundary");

template<>
template<>
void object::test<1>()
{
    input_ = fromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
    GEOSSetSRID(input_, 3857);

    result_ = GEOSBoundary(input_);
    expected_ = fromWKT("MULTILINESTRING ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");

    ensure_geometry_equals_exact(result_, expected_, 0.0);
    ensure_equals(GEOSGetSRID(input_), GEOSGetSRID(result_));
}

template<>
template<>
void object::test<2>()
{
    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(input_ != nullptr);

    result_ = GEOSBoundary(input_);
    ensure(result_ == nullptr);
}

} // namespace tut

