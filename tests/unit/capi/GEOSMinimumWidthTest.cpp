//
// Test Suite for C-API GEOSMinimumWidth
#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosminimumwidth_data : public capitest::utility {
    test_capigeosminimumwidth_data()
    {
        GEOSWKTWriter_setRoundingPrecision(wktw_, 8);
    }
};

typedef test_group<test_capigeosminimumwidth_data> group;
typedef group::object object;

group test_capigeosminimumwidth_group("capi::GEOSMinimumWidth");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    input_ = GEOSGeomFromWKT("POLYGON ((0 0, 0 15, 5 10, 5 0, 0 0))");
    ensure(nullptr != input_);

    GEOSGeometry* output = GEOSMinimumWidth(input_);
    ensure(nullptr != output);
    ensure(0 == GEOSisEmpty(output));

    wkt_ = GEOSWKTWriter_write(wktw_, output);
    ensure_equals(std::string(wkt_), std::string("LINESTRING (0 0, 5 0)"));

    GEOSGeom_destroy(output);
}

template<>
template<>
void object::test<2>
()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0,0 10, 10 10)");
    ensure(nullptr != input_);

    GEOSGeometry* output = GEOSMinimumWidth(input_);
    ensure(nullptr != output);
    ensure(0 == GEOSisEmpty(output));

    wkt_ = GEOSWKTWriter_write(wktw_, output);
    ensure_equals(std::string(wkt_), std::string("LINESTRING (5 5, 0 10)"));
    GEOSGeom_destroy(output);
}

} // namespace tut
