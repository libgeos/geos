//
// Test Suite for C-API GEOSMinimumRotatedRectangle

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
struct test_minimumrotatedrectangle_data : public capitest::utility {
    GEOSWKTWriter* wktw_;
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;
    GEOSGeometry* expected_;

    test_minimumrotatedrectangle_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr)
    {
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setOutputDimension(wktw_, 3);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        expected_ = nullptr;
    }

    ~test_minimumrotatedrectangle_data()
    {
        GEOSWKTWriter_destroy(wktw_);
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSGeom_destroy(geom3_);
        if (expected_) GEOSGeom_destroy(expected_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        expected_ = nullptr;
    }

    void checkMinRectangle(const char* wkt, const char* expected)
    {
        // input
        geom1_ = GEOSGeomFromWKT(wkt);
        ensure(nullptr != geom1_);

        // result
        geom2_ = GEOSMinimumRotatedRectangle(geom1_);
        ensure(nullptr != geom2_);

        // expected
        if (expected) {
            geom3_ = GEOSGeomFromWKT(expected);
            ensure(nullptr != geom3_);
            ensure_geometry_equals(geom2_, geom3_, 0.0001);
        }
    }
};

typedef test_group<test_minimumrotatedrectangle_data> group;
typedef group::object object;

group test_capigeosminimumrotatedrectangle_group("capi::GEOSMinimumRotatedRectangle");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    checkMinRectangle(  "POLYGON ((1 6, 6 11, 11 6, 6 1, 1 6))",
                        "POLYGON ((1 6, 6 11, 11 6, 6 1, 1 6))");
}

// zero-length
template<>
template<>
void object::test<2>
()
{
    checkMinRectangle("LINESTRING (1 1, 1 1)", "POINT (1 1)");
}

// Horizontal
template<>
template<>
void object::test<3>
()
{
    checkMinRectangle("LINESTRING (1 1, 3 1, 5 1, 7 1)", "LINESTRING (1 1, 7 1)");
}

// Vertical
template<>
template<>
void object::test<4>
()
{
    checkMinRectangle("LINESTRING (1 1, 1 4, 1 7, 1 9)", "LINESTRING (1 1, 1 9)");
}

// Bent Line
template<>
template<>
void object::test<5>
()
{
    checkMinRectangle("LINESTRING (1 2, 3 8, 9 6)", "POLYGON ((9 6, 7 10, -1 6, 1 2, 9 6))");
}

// Failure case from https://trac.osgeo.org/postgis/ticket/5163
template<>
template<>
void object::test<6>
()
{
    checkMinRectangle(
        "LINESTRING(-99.48710639268086 34.79029839231914,-99.48370699999998 34.78689899963806,-99.48152167568102 34.784713675318976)",
        "LINESTRING (-99.48710639268086 34.79029839231914, -99.48152167568102 34.784713675318976)"
        );
}

} // namespace tut
