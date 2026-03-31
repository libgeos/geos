//
// Test Suite for GEOSCoverageEdges CAPI function.

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_geoscoverageedges_data : public capitest::utility
 {
    test_geoscoverageedges_data() {}
};

typedef test_group<test_geoscoverageedges_data> group;
typedef group::object object;

group test_geoscoverageedges_data("capi::GEOSCoverageEdges");


// testTwoAdjacentInterior
template<>
template<>
void object::test<1> ()
{
    input_ = fromWKT("GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 5, 9 6, 9 1, 1 1)), POLYGON ((1 9, 6 9, 6 5, 1 6, 1 9)))");
    result_ = GEOSCoverageEdges(input_, 2); // INTERIOR
    expected_ = fromWKT("MULTILINESTRING ((1 6, 6 5))");

    ensure_geometry_equals(result_, expected_);
}

// testTwoAdjacentExterior
template<>
template<>
void object::test<2> ()
{
    input_ = fromWKT("GEOMETRYCOLLECTION (POLYGON ((1 1, 1 6, 6 5, 9 6, 9 1, 1 1)), POLYGON ((1 9, 6 9, 6 5, 1 6, 1 9)))");
    result_ = GEOSCoverageEdges(input_, 1); // EXTERIOR
    expected_ = fromWKT("MULTILINESTRING ((1 6, 1 1, 9 1, 9 6, 6 5), (1 6, 1 9, 6 9, 6 5))");

    ensure_geometry_equals(result_, expected_);
}

} // namespace tut
