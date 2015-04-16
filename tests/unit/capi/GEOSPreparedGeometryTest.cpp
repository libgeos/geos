// 
// Test Suite for C-API GEOSPreparedGeometry

#include <tut.hpp>
// geos
#include <geos_c.h>
#include <geos/io/WKBReader.h>
#include <geos/geom/PrecisionModel.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capigeospreparedgeometry_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;

        const GEOSPreparedGeometry* prepGeom1_;
        const GEOSPreparedGeometry* prepGeom2_;



        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeospreparedgeometry_data()
            : geom1_(0), geom2_(0), prepGeom1_(0), prepGeom2_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeospreparedgeometry_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
        GEOSPreparedGeom_destroy(prepGeom1_);
        GEOSPreparedGeom_destroy(prepGeom2_);
            geom1_ = 0;
            geom2_ = 0;
            prepGeom1_ = 0;
            prepGeom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeospreparedgeometry_data> group;
    typedef group::object object;

    group test_capigeospreparedgeometry_group("capi::GEOSPreparedGeometry");

    //
    // Test Cases
    //

    // Test geometry preparation
    template<>
    template<>
    void object::test<1>()
    {
            geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");
        prepGeom1_ = GEOSPrepare(geom1_);

        ensure(0 != prepGeom1_);

    }

    // Test PreparedContainsProperly
    // Taken from regress/regress_ogc_prep.sql of postgis
    // as of revno 3936 
    // ref: containsproperly200 (a)
    template<>
    template<>
    void object::test<2>()
    {
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 10, 10 10, 10 0, 0 0))");
    geom2_ = GEOSGeomFromWKT("POLYGON((2 2, 2 3, 3 3, 3 2, 2 2))");
    prepGeom1_ = GEOSPrepare(geom1_);

    ensure(0 != prepGeom1_);

    int ret = GEOSPreparedContainsProperly(prepGeom1_, geom2_);
    ensure_equals(ret, 1);

    }

    // Test PreparedContainsProperly
    // Taken from regress/regress_ogc_prep.sql of postgis
    // as of revno 3936 
    // ref: containsproperly200 (b)
    template<>
    template<>
    void object::test<3>()
    {
    geom1_ = GEOSGeomFromWKT("POLYGON((2 2, 2 3, 3 3, 3 2, 2 2))");
    geom2_ = GEOSGeomFromWKT("POLYGON((0 0, 0 10, 10 10, 10 0, 0 0))");
    prepGeom1_ = GEOSPrepare(geom1_);

    ensure(0 != prepGeom1_);

    int ret = GEOSPreparedContainsProperly(prepGeom1_, geom2_);
    ensure_equals(ret, 0);

    }

    // Test PreparedIntersects
    // Also used as a linestring leakage reported
    // by http://trac.osgeo.org/geos/ticket/305
    // 
    template<>
    template<>
    void object::test<4>()
    {
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 10)");
    geom2_ = GEOSGeomFromWKT("LINESTRING(0 10, 10 0)");
    prepGeom1_ = GEOSPrepare(geom1_);

    ensure(0 != prepGeom1_);

    int ret = GEOSPreparedIntersects(prepGeom1_, geom2_);
    ensure_equals(ret, 1);

    }

    // Test PreparedCovers
    template<>
    template<>
    void object::test<5>()
    {
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 10, 10 11, 10 0, 0 0))");
    geom2_ = GEOSGeomFromWKT("POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))");
    prepGeom1_ = GEOSPrepare(geom1_);

    ensure(0 != prepGeom1_);

    int ret = GEOSPreparedCovers(prepGeom1_, geom2_);
    ensure_equals(ret, 1);

    }

    // Test PreparedContains
    template<>
    template<>
    void object::test<6>()
    {
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 10, 10 11, 10 0, 0 0))");
    geom2_ = GEOSGeomFromWKT("POLYGON((0 0, 2 0, 2 2, 0 2, 0 0))");
    prepGeom1_ = GEOSPrepare(geom1_);

    ensure(0 != prepGeom1_);

    int ret = GEOSPreparedContains(prepGeom1_, geom2_);
    ensure_equals(ret, 1);

    }

    // Test PreparedIntersects: point on segment with FLOAT PM
    // X coordinate of 3rd and 4th vertises of the line are not 
    // float-point exact with X coordinate of the point.
    // The X values differ after 14th decimal place:
    // POINT (-23.1094689600055080 50.5195368635957180)
    // --------------------^^^^^^^------------^^^^^^^^
    // LINESTRING 3rd and 4th points
    //        -23.1094689600055150 50.5223376452201340, 
    //        -23.1094689600055010 50.5169177629559480,
    // --------------------^^^^^^^------------^^^^^^^^
    // So, in float-point precision model, the point does DOES NOT intersect the segment.
    // See RobustLineIntersectorTest.cpp for similar test cases.
    template<>
    template<>
    void object::test<7>()
    {
        // POINT located between 3rd and 4th vertex of LINESTRING
        // POINT(-23.1094689600055080 50.5195368635957180)
        std::string point("01010000009a266328061c37c0e21a172f80424940");
        // LINESTRING(-23.122057005539 50.5201976774794,-23.1153476966995 50.5133404815199,-23.1094689600055150 50.5223376452201340,-23.1094689600055010 50.5169177629559480,-23.0961967920942 50.5330464848094,-23.0887991006034 50.5258515213185,-23.0852302622362 50.5264582238409)
        std::string line("0102000000070000009909bf203f1f37c05c1d66d6954249404afe386d871d37c0a7eb1124b54149409c266328061c37c056d8bff5db42494098266328061c37c0034f7b5c2a42494060065c5aa01837c08ac001de3a4449408401b189bb1637c0b04e471a4f43494014ef84a6d11537c0b20dabfb62434940");
        geom1_ = GEOSGeomFromHEX_buf(reinterpret_cast<const unsigned char*>(line.data()), line.size());
        geom2_ = GEOSGeomFromHEX_buf(reinterpret_cast<const unsigned char*>(point.data()), point.size());

        prepGeom1_ = GEOSPrepare(geom1_);
        ensure(0 != prepGeom1_);
        int ret = GEOSPreparedIntersects(prepGeom1_, geom2_);
        ensure_equals(ret, 0);
    }

    // Test PreparedIntersects: point on segment with FIXED PM
    // X coordinate of 3rd and 4th vertises of the line are not 
    // float-point exact with X coordinate of the point.
    // The X values differ after 14th decimal place:
    // POINT (-23.1094689600055080 50.5195368635957180)
    // --------------------^^^^^^^------------^^^^^^^^
    // LINESTRING 3rd and 4th points
    //        -23.1094689600055150 50.5223376452201340, 
    //        -23.1094689600055010 50.5169177629559480,
    // --------------------^^^^^^^------------^^^^^^^^
    // So, if float-point values are trimmed up to 14 decimal digits, the point DOES intersect the segment.
    // See RobustLineIntersectorTest.cpp for similar test cases.
    template<>
    template<>
    void object::test<8>()
    {
        geos::geom::PrecisionModel pm(1e+13);
        geos::geom::GeometryFactory factory(&pm);
        geos::io::WKBReader reader(factory);

        // POINT located between 3rd and 4th vertex of LINESTRING
        // POINT(-23.1094689600055080 50.5195368635957180)
        std::string point("01010000009a266328061c37c0e21a172f80424940");
        // LINESTRING(-23.122057005539 50.5201976774794,-23.1153476966995 50.5133404815199,-23.1094689600055150 50.5223376452201340,-23.1094689600055010 50.5169177629559480,-23.0961967920942 50.5330464848094,-23.0887991006034 50.5258515213185,-23.0852302622362 50.5264582238409)
        std::string line("0102000000070000009909bf203f1f37c05c1d66d6954249404afe386d871d37c0a7eb1124b54149409c266328061c37c056d8bff5db42494098266328061c37c0034f7b5c2a42494060065c5aa01837c08ac001de3a4449408401b189bb1637c0b04e471a4f43494014ef84a6d11537c0b20dabfb62434940");
        std::stringstream sPoint(point);
        geom2_ = reinterpret_cast<GEOSGeometry*>(reader.readHEX(sPoint));
        std::stringstream sLine(line);
        geom1_ = reinterpret_cast<GEOSGeometry*>(reader.readHEX(sLine));

        prepGeom1_ = GEOSPrepare(geom1_);
        ensure(0 != prepGeom1_);
        int ret = GEOSPreparedIntersects(prepGeom1_, geom2_);
        ensure_equals(ret, 1);
    }

    // Test PreparedIntersects: point on vertex (default FLOAT PM)
    template<>
    template<>
    void object::test<9>()
    {
        // POINT located on the 3rd vertex of LINESTRING
        // POINT(-23.1094689600055 50.5223376452201)
        std::string point("01010000009c266328061c37c056d8bff5db424940");
        // LINESTRING(-23.122057005539 50.5201976774794,-23.1153476966995 50.5133404815199,-23.1094689600055 50.5223376452201,-23.1094689600055 50.5169177629559,-23.0961967920942 50.5330464848094,-23.0887991006034 50.5258515213185,-23.0852302622362 50.5264582238409)
        std::string line("0102000000070000009909bf203f1f37c05c1d66d6954249404afe386d871d37c0a7eb1124b54149409c266328061c37c056d8bff5db42494098266328061c37c0034f7b5c2a42494060065c5aa01837c08ac001de3a4449408401b189bb1637c0b04e471a4f43494014ef84a6d11537c0b20dabfb62434940");
        geom1_ = GEOSGeomFromHEX_buf(reinterpret_cast<const unsigned char*>(line.data()), line.size());
        geom2_ = GEOSGeomFromHEX_buf(reinterpret_cast<const unsigned char*>(point.data()), point.size());
        prepGeom1_ = GEOSPrepare(geom1_);
        ensure(0 != prepGeom1_);

        int ret = GEOSPreparedIntersects(prepGeom1_, geom2_);
        ensure_equals(ret, 1);
    }

    // TODO: add lots of more tests
    
} // namespace tut

