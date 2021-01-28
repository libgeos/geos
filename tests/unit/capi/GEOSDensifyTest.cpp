//
// Test Suite for C-API GEOSBuffer and GEOSBufferWithStyle

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosdensify_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;
    GEOSWKTWriter* w_;
    char* wkt_;

    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    test_capigeosdensify_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr), w_(nullptr)
    {
        initGEOS(notice, notice);
        w_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(w_, 1);
    }

    ~test_capigeosdensify_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSGeom_destroy(geom3_);
        GEOSWKTWriter_destroy(w_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        wkt_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeosdensify_data> group;
typedef group::object object;

group test_capigeosdensify_group("capi::GEOSDensify");

//
// Test Cases
//

// Densify with a tolerance slightly larger than length of all edges.
// Result should match inputs.
template<>
template<>
void object::test<1>()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
    ensure(geom1_ != nullptr);
    GEOSSetSRID(geom1_, 3857);

    geom2_ = GEOSDensify(geom1_, 10.00000000001);

    ensure("result not null", geom2_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom2_, geom1_, 0), 1);
    ensure_equals("result SRID == expected SRID", GEOSGetSRID(geom2_), GEOSGetSRID(geom1_));
}


// Densify with a tolerance equal to length of all outer edges.
// Outer edges are densified; interior ring edges are not.
template<>
template<>
void object::test<2>()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
    ensure(geom1_ != nullptr);
    GEOSSetSRID(geom1_, 3857);

    geom2_ = GEOSGeomFromWKT("POLYGON ((0 0, 5 0, 10 0, 10 5, 10 10, 5 10, 0 10, 0 5, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))");
    ensure(geom2_ != nullptr);
    GEOSSetSRID(geom2_, 3857);

    geom3_ = GEOSDensify(geom1_, 10.0);

    ensure("result not null", geom3_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom3_, geom2_, 0), 1);
    ensure_equals("result SRID == expected SRID", GEOSGetSRID(geom3_), GEOSGetSRID(geom2_));
}

// Densify with a tolerance that evenly subdivides all outer and inner edges.
template<>
template<>
void object::test<3>()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 7, 7 7, 7 1, 1 1))");
    ensure(geom1_ != nullptr);

    geom2_ = GEOSGeomFromWKT("POLYGON ((0 0, 5 0, 10 0, 10 5, 10 10, 5 10, 0 10, 0 5, 0 0), (1 1, 1 4, 1 7, 4 7, 7 7, 7 4, 7 1, 4 1, 1 1))");
    ensure(geom2_ != nullptr);

    geom3_ = GEOSDensify(geom1_, 5.00000000001);

    ensure("result not null", geom3_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom3_, geom2_, 0), 1);
}

// Densify a LINESTRING
template<>
template<>
void object::test<4>()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 6 6)");
    ensure(geom1_ != nullptr);
    GEOSSetSRID(geom1_, 3857);

    geom2_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 2, 4 4, 6 6)");
    ensure(geom2_ != nullptr);
    GEOSSetSRID(geom2_, 3857);

    geom3_ = GEOSDensify(geom1_, 3.0);

    ensure("result not null", geom2_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom3_, geom2_, 0), 1);
    ensure_equals("result SRID == expected SRID", GEOSGetSRID(geom3_), GEOSGetSRID(geom2_));
}

// Densify a LINEARRING
template<>
template<>
void object::test<5>()
{
    geom1_ = GEOSGeomFromWKT("LINEARRING (0 0, 0 6, 6 6, 0 0)");
    ensure(geom1_ != nullptr);
    GEOSSetSRID(geom1_, 3857);

    geom2_ = GEOSGeomFromWKT("LINEARRING (0 0, 0 2, 0 4, 0 6, 2 6, 4 6, 6 6, 4 4, 2 2, 0 0)");
    ensure(geom2_ != nullptr);
    GEOSSetSRID(geom2_, 3857);

    geom3_ = GEOSDensify(geom1_, 3.0);

    ensure("result not null", geom2_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom3_, geom2_, 0), 1);
    ensure_equals("result SRID == expected SRID", GEOSGetSRID(geom3_), GEOSGetSRID(geom2_));
}

// Densify a POINT
// Results should match inputs
template<>
template<>
void object::test<6>()
{
    geom1_ = GEOSGeomFromWKT("POINT (0 0)");
    ensure(geom1_ != nullptr);
    GEOSSetSRID(geom1_, 3857);

    geom2_ = GEOSDensify(geom1_, 3.0);

    ensure("result not null", geom2_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom2_, geom1_, 0), 1);
    ensure_equals("result SRID == expected SRID", GEOSGetSRID(geom2_), GEOSGetSRID(geom1_));
}

// Densify a MULTIPOINT
// Results should match inputs
template<>
template<>
void object::test<7>()
{
    geom1_ = GEOSGeomFromWKT("MULTIPOINT ((0 0), (10 10))");
    ensure(geom1_ != nullptr);
    GEOSSetSRID(geom1_, 3857);

    geom2_ = GEOSDensify(geom1_, 3.0);

    ensure("result not null", geom2_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom2_, geom1_, 0), 1);
    ensure_equals("result SRID == expected SRID", GEOSGetSRID(geom2_), GEOSGetSRID(geom1_));
}

// Densify an empty polygon
// Results should match inputs
template<>
template<>
void object::test<8>()
{
    geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure(geom1_ != nullptr);

    geom2_ = GEOSDensify(geom1_, 3.0);

    ensure("result not null", geom2_ != nullptr);
    ensure_equals("result == expected", GEOSEqualsExact(geom2_, geom1_, 0), 1);
}

// Densify a null geometry
// Result should be null
template<>
template<>
void object::test<9>()
{
    geom2_ = GEOSDensify(nullptr, 3.0);
    ensure("result is null", geom2_ == nullptr);
}

// Densify with an invalid tolerances should fail
// Note: this raises "IllegalArgumentException: Tolerance must be positive:
template<>
template<>
void object::test<10>()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    ensure(geom1_ != nullptr);

    geom2_ = GEOSDensify(geom1_, 0.0);
    ensure(geom2_ == nullptr);

    geom2_ = GEOSDensify(geom1_, -1.0);
    ensure(geom2_ == nullptr);
}


} // namespace tut

