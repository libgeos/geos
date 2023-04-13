//
// Test Suite for C-API GEOSCoverageUnion

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capicoveragesimplify_data : public capitest::utility {

    test_capicoveragesimplify_data() {
    }

    ~test_capicoveragesimplify_data() {
    }

};


typedef test_group<test_capicoveragesimplify_data> group;
typedef group::object object;

group test_capicoveragesimplify_group("capi::GEOSCoverageSimplify");

//
// Test Cases
//


template<>
template<> void object::test<1>
()
{
    const char* inputWKT = "GEOMETRYCOLLECTION(POLYGON ((100 100, 200 200, 300 100, 200 101, 100 100)), POLYGON ((150 0, 100 100, 200 101, 300 100, 250 0, 150 0)))";

    input_ = fromWKT(inputWKT);
    result_ = GEOSCoverageSimplifyVW(input_, 10.0, 0);

    ensure( result_ != nullptr );
    ensure( GEOSGeomTypeId(result_) == GEOS_GEOMETRYCOLLECTION );

    const char* expectedWKT = "GEOMETRYCOLLECTION(POLYGON ((100 100, 200 200, 300 100, 100 100)), POLYGON ((150 0, 100 100, 300 100, 250 0, 150 0)))";

    expected_ = fromWKT(expectedWKT);

    // std::cout << toWKT(result_) << std::endl;
    // std::cout << toWKT(expected_) << std::endl;

    ensure_geometry_equals(result_, expected_, 0.1);
}



} // namespace tut

