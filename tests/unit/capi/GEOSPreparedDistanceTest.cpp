//
// Test Suite for C-API GEOSPreparedDistance

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosprepareddistance_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    const GEOSPreparedGeometry* pgeom1_;

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

    test_capigeosprepareddistance_data()
        : geom1_(nullptr), geom2_(nullptr), pgeom1_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capigeosprepareddistance_data()
    {
        GEOSGeom_destroy(geom2_);
        GEOSPreparedGeom_destroy(pgeom1_);
        GEOSGeom_destroy(geom1_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom1_ = nullptr;
        finishGEOS();
    }

    void checkDistance(const char* wkt1, const char* wkt2,
                       double dist, double tol=0)
    {
        geom1_ = GEOSGeomFromWKT(wkt1);
        ensure(nullptr != geom1_);
        pgeom1_ = GEOSPrepare(geom1_);
        ensure(nullptr != pgeom1_);
        geom2_ = GEOSGeomFromWKT(wkt2);
        ensure(nullptr != geom2_);


        double obt_dist;
        int ret = GEOSPreparedDistance(pgeom1_, geom2_, &obt_dist);
        ensure_equals("return code", ret, 1);
        ensure_equals("distance", obt_dist, dist, tol);

    }


};

typedef test_group<test_capigeosprepareddistance_data> group;
typedef group::object object;

group test_capigeosprepareddistance_group("capi::GEOSPreparedDistance");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    checkDistance(
        "POLYGON EMPTY",
        "POLYGON EMPTY",
        std::numeric_limits<double>::infinity()
    );
}

template<>
template<>
void object::test<2>
()
{
    checkDistance(
        "POLYGON((1 1,1 5,5 5,5 1,1 1))",
        "POLYGON((8 8, 9 9, 9 10, 8 8))",
        4.242640687119285, 1e-12
    );

}

template<>
template<>
void object::test<3>
()
{
    checkDistance(
        "POLYGON((1 1,1 5,5 5,5 1,1 1))",
        "POINT(2 2)",
        0
    );
}

template<>
template<>
void object::test<4>
()
{
    checkDistance(
        "LINESTRING(1 5,5 5,5 1,1 1)",
        "POINT(2 2)",
        1
    );
}

template<>
template<>
void object::test<5>
()
{
    checkDistance(
        "LINESTRING(0 0,10 10)",
        "LINESTRING(0 10,10 0)",
        0
    );
}

template<>
template<>
void object::test<6>
()
{
    checkDistance(
        "POLYGON((0 0,10 0,10 10,0 10,0 0))",
        "LINESTRING(8 5,12 5)",
        0
    );
}

template<>
template<>
void object::test<7>
()
{
    checkDistance(
        "LINESTRING EMPTY",
        "POINT EMPTY",
        std::numeric_limits<double>::infinity()
    );
}

template<>
template<>
void object::test<8>
()
{
    checkDistance(
        "POINT EMPTY",
        "LINESTRING EMPTY",
        std::numeric_limits<double>::infinity()
    );
}

} // namespace tut

