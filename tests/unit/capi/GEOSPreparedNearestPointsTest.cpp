// $Id: GEOSPreparedNearestPointsTest.cpp 2424 2009-04-29 23:52:36Z mloskot $
//
// Test Suite for C-API GEOSPreparedNearestPoints

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
struct test_capigeospreparednearestpoints_data {
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

    test_capigeospreparednearestpoints_data()
        : geom1_(nullptr), geom2_(nullptr), pgeom1_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capigeospreparednearestpoints_data()
    {
        GEOSGeom_destroy(geom2_);
        GEOSPreparedGeom_destroy(pgeom1_);
        GEOSGeom_destroy(geom1_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom1_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeospreparednearestpoints_data> group;
typedef group::object object;

group test_capigeospreparednearestpoints_group("capi::GEOSPreparedNearestPoints");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");
    pgeom1_ = GEOSPrepare(geom1_);
    geom2_ = GEOSGeomFromWKT("POLYGON EMPTY");

    ensure(nullptr != pgeom1_);
    ensure(nullptr != geom2_);

    GEOSCoordSequence* coords_;
    coords_ = GEOSPreparedNearestPoints(pgeom1_, geom2_);

    ensure(nullptr == coords_);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((1 1,1 5,5 5,5 1,1 1))");
    pgeom1_ = GEOSPrepare(geom1_);
    geom2_ = GEOSGeomFromWKT("POLYGON((8 8, 9 9, 9 10, 8 8))");

    ensure(nullptr != pgeom1_);
    ensure(nullptr != geom2_);

    GEOSCoordSequence* coords_;
    coords_ = GEOSPreparedNearestPoints(pgeom1_, geom2_);

    ensure(nullptr != coords_);

    unsigned int size;
    GEOSCoordSeq_getSize(coords_, &size);
    ensure_equals(size, 2u);

    double  x1, x2, y1, y2;

    /* Point in pgeom1_
     */
    GEOSCoordSeq_getOrdinate(coords_, 0, 0, &x1);
    GEOSCoordSeq_getOrdinate(coords_, 0, 1, &y1);
    ensure_equals(x1, 5);
    ensure_equals(y1, 5);

    /* Point in geom2_
     */
    GEOSCoordSeq_getOrdinate(coords_, 1, 0, &x2);
    GEOSCoordSeq_getOrdinate(coords_, 1, 1, &y2);
    ensure_equals(x2, 8);
    ensure_equals(y2, 8);

    GEOSCoordSeq_destroy(coords_);
}

template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((1 1,1 5,5 5,5 1,1 1))");
    pgeom1_ = GEOSPrepare(geom1_);
    geom2_ = GEOSGeomFromWKT("POINT(2 2)");

    ensure(nullptr != geom1_);
    ensure(nullptr != geom2_);

    GEOSCoordSequence* coords_;
    coords_ = GEOSPreparedNearestPoints(pgeom1_, geom2_);

    ensure(nullptr != coords_);

    unsigned int size;
    GEOSCoordSeq_getSize(coords_, &size);
    ensure_equals(size, 2u);

    double  x1, x2, y1, y2;

    /* Point in geom1_
     */
    GEOSCoordSeq_getOrdinate(coords_, 0, 0, &x1);
    GEOSCoordSeq_getOrdinate(coords_, 0, 1, &y1);
    ensure_equals(x1, 2);
    ensure_equals(y1, 2);

    /* Point in geom2_
     */
    GEOSCoordSeq_getOrdinate(coords_, 1, 0, &x2);
    GEOSCoordSeq_getOrdinate(coords_, 1, 1, &y2);
    ensure_equals(x2, 2);
    ensure_equals(y2, 2);

    GEOSCoordSeq_destroy(coords_);
}

template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(1 5,5 5,5 1,1 1)");
    pgeom1_ = GEOSPrepare(geom1_);
    geom2_ = GEOSGeomFromWKT("POINT(2 2)");

    ensure(nullptr != geom1_);
    ensure(nullptr != geom2_);

    GEOSCoordSequence* coords_;
    coords_ = GEOSPreparedNearestPoints(pgeom1_, geom2_);

    ensure(nullptr != coords_);

    unsigned int size;
    GEOSCoordSeq_getSize(coords_, &size);
    ensure_equals(size, 2u);

    double  x1, x2, y1, y2;

    /* Point in geom1_
     */
    GEOSCoordSeq_getOrdinate(coords_, 0, 0, &x1);
    GEOSCoordSeq_getOrdinate(coords_, 0, 1, &y1);
    ensure_equals(x1, 2);
    ensure_equals(y1, 1);


    /* Point in geom2_
     */
    GEOSCoordSeq_getOrdinate(coords_, 1, 0, &x2);
    GEOSCoordSeq_getOrdinate(coords_, 1, 1, &y2);
    ensure_equals(x2, 2);
    ensure_equals(y2, 2);

    GEOSCoordSeq_destroy(coords_);
}

} // namespace tut

