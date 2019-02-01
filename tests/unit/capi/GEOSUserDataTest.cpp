//
// Test Suite for C-API GEOSSimplify

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeouserdata_data {
    GEOSGeometry* geom_;

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

    test_capigeouserdata_data()
        : geom_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capigeouserdata_data()
    {
        GEOSGeom_destroy(geom_);
        geom_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeouserdata_data> group;
typedef group::object object;

group test_capigeosuserdata_group("capi::GEOSUserData");

//
// Test Cases
//

// Test GEOSUserData
template<>
template<>
void object::test<1>
()
{
    geom_ = GEOSGeomFromWKT("POLYGON EMPTY");

    ensure(0 != GEOSisEmpty(geom_));

    int in = 1;
    GEOSGeom_setUserData(geom_, &in);

    int* out = (int*)GEOSGeom_getUserData(geom_);

    ensure_equals(*out, 1);
}

struct UserData {
    int a;
    int b;
} ;

template<>
template<>
void object::test<2>
()
{
    geom_ = GEOSGeomFromWKT("POLYGON EMPTY");

    ensure(0 != GEOSisEmpty(geom_));

    UserData in;
    in.a = 1;
    in.b = 2;

    GEOSGeom_setUserData(geom_, &in);

    struct UserData* out = (struct UserData*)GEOSGeom_getUserData(geom_);

    ensure_equals(in.a, out->a);
    ensure_equals(in.b, out->b);
    ensure_equals(&in, out);
}

} // namespace tut

