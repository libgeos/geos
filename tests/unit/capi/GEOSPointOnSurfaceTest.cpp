// $Id$
//
// Test Suite for C-API GEOSPointOnSurface

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
struct test_capipointonsurface_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
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

    test_capipointonsurface_data()
        : geom1_(nullptr), geom2_(nullptr), wkt_(nullptr)
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setRoundingPrecision(wktw_, 8);
    }

    ~test_capipointonsurface_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSWKTWriter_destroy(wktw_);
        GEOSFree(wkt_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        wkt_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capipointonsurface_data> group;
typedef group::object object;

group test_capipointonsurface_group("capi::GEOSPointOnSurface");

//
// Test Cases
//

// Single point
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(10 0)");

    ensure(nullptr != geom1_);

    geom2_ = GEOSPointOnSurface(geom1_);

    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("POINT (10 0)"));

}

// line
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 5 0, 10 0)");

    ensure(nullptr != geom1_);

    geom2_ = GEOSPointOnSurface(geom1_);

    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("POINT (5 0)"));

}

// polygon
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");

    ensure(nullptr != geom1_);

    geom2_ = GEOSPointOnSurface(geom1_);

    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("POINT (5 5)"));

}

// Tiny triangle, see http://trac.osgeo.org/geos/ticket/559
template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT(
                 "POLYGON(( \
56.528666666700 25.2101666667, \
56.529000000000 25.2105000000, \
56.528833333300 25.2103333333, \
56.528666666700 25.2101666667))");

    ensure(nullptr != geom1_);

    geom2_ = GEOSPointOnSurface(geom1_);

    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("POINT (56.528667 25.210167)"));

}

// Empty geometry -- see http://trac.osgeo.org/geos/ticket/560
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");

    ensure(nullptr != geom1_);

    geom2_ = GEOSPointOnSurface(geom1_);

    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("POINT EMPTY"));
}

// Single point linestring -- see http://trac.osgeo.org/geos/ticket/609
template<>
template<>
void object::test<6>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 0 0)");

    ensure(nullptr != geom1_);

    geom2_ = GEOSPointOnSurface(geom1_);

    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("POINT (0 0)"));

}

// Check point on surface for three similar polygons (shapes)
// https://trac.osgeo.org/geos/ticket/840
template<>
template<>
void object::test<7>
()
{
    // Polygon A
    geom1_ = GEOSGeomFromWKT("POLYGON(("
                             "182111.031940953 141935.935903267,181944.315698016 141813.663222482,"
                             "181993.426552077 141735.161360171,182025.215323227 141755.058841504,"
                             "182042.126877935 141723.338145732,182089.035943744 141755.453352846,"
                             "182096.699289843 141744.892410235,182142.107084301 141773.967727472,"
                             "182140.268233846 141825.482203511,182123.233970367 141827.369156254,"
                             "182111.031940953 141935.935903267))");
    ensure(nullptr != geom1_);
    geom2_ = GEOSPointOnSurface(geom1_);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    ensure_equals(std::string(wkt_), std::string("POINT (182077.08 141881.65)"));
}

template<>
template<>
void object::test<8>
()
{
    // Polygon B from ticket #840
    geom1_ = GEOSGeomFromWKT("POLYGON(("
                             "182512.231897141 141935.935903267,182344.315698016 141813.663222482,"
                             "182393.426552077 141735.161360171,182425.215323227 141755.058841504,"
                             "182442.126877935 141723.338145732,182489.035943744 141755.453352846,"
                             "182496.699289843 141744.892410235,182542.107084301 141773.967727472,"
                             "182540.268233846 141825.482203511,182522.034014178 141825.369229273,"
                             "182512.231897141 141935.935903267))");
    ensure(nullptr != geom1_);
    geom2_ = GEOSPointOnSurface(geom1_);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    ensure_equals(std::string(wkt_), std::string("POINT (182476.76 141880.71)"));
}

template<>
template<>
void object::test<9>
()
{
    // Polygon C from ticket #840
    geom1_ = GEOSGeomFromWKT("POLYGON(("
                             "182635.760119718 141846.477712277,182826.153168283 141974.473039044,"
                             "182834.952846998 141857.67730337,182862.151853936 141851.277537031,"
                             "182860.551912351 141779.280165725,182824.553226698 141748.881275618,"
                             "182814.953577191 141758.480925126,182766.155358861 141721.682268681,"
                             "182742.156235092 141744.881421657,182692.558045971 141716.882443927,"
                             "182635.760119718 141846.477712277))");
    ensure(nullptr != geom1_);
    geom2_ = GEOSPointOnSurface(geom1_);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);
    ensure_equals(std::string(wkt_), std::string("POINT (182755.89 141812.88)"));
}

} // namespace tut

