// $Id: GEOSNearestPointsTest.cpp 2424 2009-04-29 23:52:36Z mloskot $
//
// Test Suite for C-API GEOSNearestPoints

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosnearestpoints_data : public capitest::utility {
    void checkNearestPoints(const char* wkt1, const char* wkt2,
                            double x1, double y1,
                            double x2, double y2)
    {
        geom1_ = GEOSGeomFromWKT(wkt1);
        ensure(nullptr != geom1_);
        geom2_ = GEOSGeomFromWKT(wkt2);
        ensure(nullptr != geom2_);
        GEOSCoordSequence* coords_ = GEOSNearestPoints(geom1_, geom2_);

        unsigned int size;
        GEOSCoordSeq_getSize(coords_, &size);
        ensure_equals("CoordSeq size", size, 2u);

        double  ox, oy;

        /* Point in geom1_ */
        GEOSCoordSeq_getOrdinate(coords_, 0, 0, &ox);
        GEOSCoordSeq_getOrdinate(coords_, 0, 1, &oy);
        ensure_equals("P1 x", ox, x1);
        ensure_equals("P1 y", oy, y1);

        /* Point in geom2_ */
        GEOSCoordSeq_getOrdinate(coords_, 1, 0, &ox);
        GEOSCoordSeq_getOrdinate(coords_, 1, 1, &oy);
        ensure_equals("P2 x", ox, x2);
        ensure_equals("P2 y", oy, y2);

        GEOSCoordSeq_destroy(coords_);
    }

    void checkNearestPointsNull(const char* wkt1, const char* wkt2)
    {
        geom1_ = GEOSGeomFromWKT(wkt1);
        ensure(nullptr != geom1_);
        geom2_ = GEOSGeomFromWKT(wkt2);
        ensure(nullptr != geom2_);
        GEOSCoordSequence* coords_ = GEOSNearestPoints(geom1_, geom2_);

        ensure(nullptr == coords_);
    }

};

typedef test_group<test_capigeosnearestpoints_data> group;
typedef group::object object;

group test_capigeosnearestpoints_group("capi::GEOSNearestPoints");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    checkNearestPointsNull("POLYGON EMPTY", "POLYGON EMPTY");
}

template<>
template<>
void object::test<2>
()
{
    set_test_name("closest points are vertices of two disjoint inputs");

    checkNearestPoints(
        "POLYGON((1 1,1 5,5 5,5 1,1 1))",
        "POLYGON((8 8, 9 9, 9 10, 8 8))",
        5, 5, 8, 8
    );

}

template<>
template<>
void object::test<3>
()
{
    set_test_name("point inside polygon");

    checkNearestPoints(
        "POLYGON((1 1,1 5,5 5,5 1,1 1))",
        "POINT(2 2)",
        2, 2, 2, 2
    );

}

template<>
template<>
void object::test<4>
()
{
    set_test_name("closest point is not a vertex of LineString");

    checkNearestPoints(
        "LINESTRING(1 5,5 5,5 1,1 1)",
        "POINT(2 2)",
        2, 1, 2, 2
    );
}

template<>
template<>
void object::test<5>
()
{
    set_test_name("two crossing LineStrings");

    checkNearestPoints(
        "LINESTRING(0 0,10 10)",
        "LINESTRING(0 10,10 0)",
        5, 5, 5, 5
    );
}

template<>
template<>
void object::test<6>
()
{
    set_test_name("LineString partially inside Polygon");

    checkNearestPoints(
        "POLYGON((0 0,10 0,10 10,0 10,0 0))",
        "LINESTRING(8 5,12 5)",
        /* But could also be the intersection point... */
        8, 5, 8, 5
    );
}

template<>
template<>
void object::test<7>() {
    set_test_name("2D points returned for 4D inputs");

    geom1_ = fromWKT("POINT ZM (0 0 1 2)");
    geom2_ = fromWKT("POINT ZM (3 4 7 9)");

    ensure(geom1_);
    ensure(geom2_);

    GEOSCoordSequence* coords = GEOSNearestPoints(geom1_, geom2_);
    ensure(coords);
    ensure(!GEOSCoordSeq_hasM(coords));
    ensure(!GEOSCoordSeq_hasZ(coords));

    GEOSCoordSeq_destroy(coords);
}

template<>
template<>
void object::test<8>
()
{
    set_test_name("curved inputs");

    useContext();

    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (3 0, 4 0)");

    GEOSCoordSequence* coords = GEOSNearestPoints_r(ctxt_, geom1_, geom2_);
    ensure(coords == nullptr);

    useCurveConversion();

    coords = GEOSNearestPoints_r(ctxt_, geom1_, geom2_);
    ensure(coords != nullptr);

    unsigned int size;
    ensure(GEOSCoordSeq_getSize_r(ctxt_, coords, &size));
    ensure_equals(size, 2u);

    double x1, y1, x2, y2;
    GEOSCoordSeq_getXY_r(ctxt_, coords, 0, &x1, &y1);
    GEOSCoordSeq_getXY_r(ctxt_, coords, 1, &x2, &y2);

    ensure_equals(x1, 2);
    ensure_equals(y1, 0);
    ensure_equals(x2, 3);
    ensure_equals(y2, 0);

    GEOSCoordSeq_destroy_r(ctxt_, coords);
}

} // namespace tut

