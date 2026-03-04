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
        ensure_geometry_equals_identical(result_, expected_);
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

template<>
template<>
void object::test<5>()
{
    set_test_name("POINT ZM");

    checkEnvelope(
        "POINT ZM (1 2 3 4)",
             "POINT (1 2)"
        );
}

template<>
template<>
void object::test<6>()
{
    set_test_name("LINESTRING ZM");

    checkEnvelope(
        "LINESTRING ZM (0 0 3 4, 5 0 7 8)",
        "POLYGON ((0 0, 5 0, 5 0, 0 0, 0 0))" // collapsed polygon
    );
}

template<>
template<>
void object::test<7>()
{
    set_test_name("POLYGON ZM");

    checkEnvelope(
        "POLYGON ZM ((0 0 1 2, 1 0 3 4, 1 1 5 6, 0 0 1 2))",
        "POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))" // collapsed polygon
    );
}

template<>
template<>
void object::test<9>()
{
    set_test_name("curved inputs");

    checkEnvelope("CIRCULARSTRING (0 0, 1 1, 2 0)",
        "POLYGON ((0 0, 2 0, 2 1, 0 1, 0 0))");
}

} // namespace tut

