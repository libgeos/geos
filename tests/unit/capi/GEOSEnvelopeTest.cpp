#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosenvelope_data : public capitest::utility
{
    void
    checkEnvelope(const char* wktIn, const char* wktExp)
    {
        input_ = fromWKT(wktIn);
        result_ = GEOSEnvelope(input_);
        expected_ = fromWKT(wktExp);
        ensure_geometry_equals(result_, expected_, 0);
    }

};

typedef test_group<test_geosenvelope_data> group;
typedef group::object object;

group test_geosenvelope("capi::GEOSEnvelope");

// non-degenerate input
template<>
template<>
void object::test<1>()
{
    checkEnvelope(
        "LINESTRING (1 2, 4 5, 9 -2)",
        "POLYGON ((1 -2, 9 -2, 9 5, 1 5, 1 -2))"
        );
}

// point input
template<>
template<>
void object::test<2>()
{
    checkEnvelope(
        "POINT (3 8)",
        "POINT (3 8)");
}

// empty point input
template<>
template<>
void object::test<3>()
{
    checkEnvelope(
        "POINT EMPTY",
        "POINT EMPTY");
}

// empty polygon input
template<>
template<>
void object::test<4>()
{
    checkEnvelope(
        "POLYGON EMPTY",
        "POINT EMPTY");
}

} // namespace tut

