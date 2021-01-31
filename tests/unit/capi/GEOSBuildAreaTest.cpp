// Test Suite for C-API BuildArea

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capi_buildarea_data : public capitest::utility {
    GEOSWKTWriter* wktw_;
    GEOSGeometry* geom1_ = nullptr;
    GEOSGeometry* geom2_ = nullptr;

    std::string
    toWKT(GEOSGeometry* g)
    {
        char* wkt = GEOSWKTWriter_write(wktw_, g);
        std::string ret(wkt);
        GEOSFree(wkt);
        return ret;
    }

    test_capi_buildarea_data()
    {
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setOutputDimension(wktw_, 3);
    }

    ~test_capi_buildarea_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSWKTWriter_destroy(wktw_);
    }

};

typedef test_group<test_capi_buildarea_data> group;
typedef group::object object;

group test_capi_buildarea_group("capi::GEOSBuildArea");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION(LINESTRING(0 0,0 1,1 1),LINESTRING(1 1,1 0,0 0))");
    geom2_ = GEOSBuildArea(geom1_);
    ensure_equals(toWKT(geom2_), std::string("POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))"));
}
} // namespace tut
