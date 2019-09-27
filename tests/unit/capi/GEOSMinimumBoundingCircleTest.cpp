// $Id$
//
// Test Suite for C-API GEOSGetCentroid

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
struct test_capiminimumboundingcircle_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;
    GEOSWKTWriter* wktw_;
    char* wkt_;
    double area_;

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

    test_capiminimumboundingcircle_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr), wkt_(nullptr)
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setRoundingPrecision(wktw_, 8);
    }

    ~test_capiminimumboundingcircle_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSGeom_destroy(geom3_);
        GEOSWKTWriter_destroy(wktw_);
        GEOSFree(wkt_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        wkt_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capiminimumboundingcircle_data> group;
typedef group::object object;

group test_capiminimumboundingcircle_group("capi::GEOSMinimumBoundingCircle");

//
// Test Cases
//

// Single point
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(10 10)");
    ensure(nullptr != geom1_);
    geom2_ = GEOSMinimumBoundingCircle(geom1_, NULL, NULL);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("POINT (10 10)"));
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 10, 0 20)");
    ensure(nullptr != geom1_);
    geom2_ = GEOSMinimumBoundingCircle(geom1_, NULL, NULL);
    ensure(nullptr != geom2_);
    geom3_ = GEOSGeomFromWKT("POLYGON ((5 15, 4.9039264 14.024548, 4.6193977 13.086583, 4.1573481 12.222149, 3.5355339 11.464466, 2.7778512 10.842652, 1.9134172 10.380602, 0.97545161 10.096074, 8.0777229e-15 10, -0.97545161 10.096074, -1.9134172 10.380602, -2.7778512 10.842652, -3.5355339 11.464466, -4.1573481 12.222149, -4.6193977 13.086583, -4.9039264 14.024548, -5 15, -4.9039264 15.975452, -4.6193977 16.913417, -4.1573481 17.777851, -3.5355339 18.535534, -2.7778512 19.157348, -1.9134172 19.619398, -0.97545161 19.903926, -1.8682053e-14 20, 0.97545161 19.903926, 1.9134172 19.619398, 2.7778512 19.157348, 3.5355339 18.535534, 4.1573481 17.777851, 4.6193977 16.913417, 4.9039264 15.975452, 5 15))");
    ensure(nullptr != geom3_);

    // wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    bool eq = GEOSEqualsExact(geom2_, geom3_, 0.0001) != 0;
    ensure(eq);
}

template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    ensure(nullptr != geom1_);
    geom2_ = GEOSMinimumBoundingCircle(geom1_, NULL, NULL);
    ensure(nullptr != geom2_);
    geom3_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure(nullptr != geom3_);

    // wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    // std::cout << wkt_ << std::endl;

    bool eq = GEOSEqualsExact(geom2_, geom3_, 0.0001) != 0;
    ensure(eq);
}

template<>
template<>
void object::test<4>
()
{
    GEOSGeometry* center;
    GEOSGeometry* center_expected;
    double radius;
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 10, 0 20)");
    ensure(nullptr != geom1_);
    geom2_ = GEOSMinimumBoundingCircle(geom1_, &radius, &center);
    ensure(nullptr != geom2_);
    geom3_ = GEOSGeomFromWKT("POLYGON ((5 15, 4.9039264 14.024548, 4.6193977 13.086583, 4.1573481 12.222149, 3.5355339 11.464466, 2.7778512 10.842652, 1.9134172 10.380602, 0.97545161 10.096074, 8.0777229e-15 10, -0.97545161 10.096074, -1.9134172 10.380602, -2.7778512 10.842652, -3.5355339 11.464466, -4.1573481 12.222149, -4.6193977 13.086583, -4.9039264 14.024548, -5 15, -4.9039264 15.975452, -4.6193977 16.913417, -4.1573481 17.777851, -3.5355339 18.535534, -2.7778512 19.157348, -1.9134172 19.619398, -0.97545161 19.903926, -1.8682053e-14 20, 0.97545161 19.903926, 1.9134172 19.619398, 2.7778512 19.157348, 3.5355339 18.535534, 4.1573481 17.777851, 4.6193977 16.913417, 4.9039264 15.975452, 5 15))");
    ensure(nullptr != geom3_);
    center_expected = GEOSGeomFromWKT("POINT(0 15)");

    // wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    // std::cout << wkt_ << std::endl;

    bool eq;
    eq = GEOSEqualsExact(geom2_, geom3_, 0.0001) != 0;
    ensure(eq);

    eq = GEOSEqualsExact(center, center_expected, 0.0001) != 0;
    ensure(eq);
    GEOSGeom_destroy(center);
    GEOSGeom_destroy(center_expected);

    ensure(fabs(radius) - 5.0 < 0.001);
}

} // namespace tut

