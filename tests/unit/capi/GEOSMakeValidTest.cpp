// Test Suite for C-API MakeValid

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdint>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capimakevalid_data {
    GEOSWKTWriter* wktw_;
    GEOSGeometry* geom1_ = nullptr;
    GEOSGeometry* geom2_ = nullptr;
    GEOSGeometry* expect_ = nullptr;

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

    std::string
    toWKT(GEOSGeometry* g)
    {
        char* wkt = GEOSWKTWriter_write(wktw_, g);
        std::string ret(wkt);
        GEOSFree(wkt);
        return ret;
    }

    test_capimakevalid_data()
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setOutputDimension(wktw_, 3);
    }

    ~test_capimakevalid_data()
    {
        if (geom1_) GEOSGeom_destroy(geom1_);
        if (geom2_) GEOSGeom_destroy(geom2_);
        if (expect_) GEOSGeom_destroy(expect_);
        GEOSWKTWriter_destroy(wktw_);
        finishGEOS();
    }

};

typedef test_group<test_capimakevalid_data> group;
typedef group::object object;

group test_capimakevalid_group("capi::GEOSMakeValid");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0,1 1,0 1,1 0,0 0))");
    geom2_ = GEOSMakeValid(geom1_);
    expect_ = GEOSGeomFromWKT("MULTIPOLYGON (((0 0, 0.5 0.5, 1 0, 0 0)), ((0.5 0.5, 0 1, 1 1, 0.5 0.5)))");
    GEOSNormalize(geom2_);
    GEOSNormalize(expect_);
    ensure(GEOSEqualsExact(geom2_, expect_, 0.01));
}

template<>
template<>
void object::test<2>
()
{
    const char* hex = "0103000000010000000900000062105839207df640378941e09d491c41ced67431387df640c667e7d398491c4179e92631387df640d9cef7d398491c41fa7e6abcf87df640cdcccc4c70491c41e3a59bc4527df64052b81e053f491c41cdcccccc5a7ef640e3a59bc407491c4104560e2da27df640aaf1d24dd3481c41e9263108c67bf64048e17a1437491c4162105839207df640378941e09d491c41";
    geom1_ = GEOSGeomFromHEX_buf((uint8_t*)hex, std::strlen(hex));
    geom2_ = GEOSMakeValid(geom1_);
    // std::cout << toWKT(geom2_) << std::endl;
    expect_ = GEOSGeomFromWKT("POLYGON ((92127.546 463452.075, 92117.173 463439.755, 92133.675 463425.942, 92122.136 463412.826, 92092.37699999999 463437.77, 92114.014 463463.469, 92115.512 463462.207, 92115.51207431706 463462.2069374289, 92127.546 463452.075))");
    GEOSNormalize(geom2_);
    GEOSNormalize(expect_);
    ensure(GEOSEqualsExact(geom2_, expect_, 0.01));
}





} // namespace tut
