#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoswktwriter_data : public capitest::utility {

    test_geoswktwriter_data() :
        wktwriter_(GEOSWKTWriter_create()),
        wkt_(nullptr)
    {}

    ~test_geoswktwriter_data() {
        GEOSWKTWriter_destroy(wktwriter_);
        GEOSFree(wkt_);
    }

    void
    test_writer_wkt(std::string const& wkt)
    {
        GEOSGeometry* geom1 = GEOSGeomFromWKT(wkt.c_str());
        ensure(nullptr != geom1);

        char* wkt_c = GEOSWKTWriter_write(wktwriter_, geom1);
        std::string out(wkt_c);
        free(wkt_c);
        GEOSGeom_destroy(geom1);

        ensure_equals(out, wkt);
    }

    void
    test_writer_wkt(std::string const& wkt, std::string const& expected)
    {
        GEOSGeometry* geom1 = GEOSGeomFromWKT(wkt.c_str());
        ensure(nullptr != geom1);

        char* wkt_c = GEOSWKTWriter_write(wktwriter_, geom1);
        std::string out(wkt_c);
        free(wkt_c);
        GEOSGeom_destroy(geom1);

        ensure_equals(out, expected);
    }

    GEOSWKTWriter* wktwriter_;
    char* wkt_;
};

typedef test_group<test_geoswktwriter_data> group;
typedef group::object object;

group test_geoswktwriter("capi::GEOSWKTWriter");

// Check default output dimension 4
template<>
template<>
void object::test<1>()
{
    ensure_equals(GEOSWKTWriter_getOutputDimension(wktwriter_), 4);

    test_writer_wkt("POINT (10 13)");
    test_writer_wkt("POINT Z (10 13 3)");
    test_writer_wkt("POINT M (10 13 5)");
    test_writer_wkt("POINT ZM (10 13 3 5)");
}

// Check writer with output dimension 2
template<>
template<>
void object::test<2>()
{
    GEOSWKTWriter_setOutputDimension(wktwriter_, 2);
    ensure_equals("getOutputDimension_2", GEOSWKTWriter_getOutputDimension(wktwriter_), 2);

    test_writer_wkt("POINT (10 13)");
    test_writer_wkt("POINT Z (10 13 3)", "POINT (10 13)");
    test_writer_wkt("POINT M (10 13 5)", "POINT (10 13)");
    test_writer_wkt("POINT ZM (10 13 3 5)", "POINT (10 13)");
}

// Check writer with output dimension 3
template<>
template<>
void object::test<3>()
{
    GEOSWKTWriter_setTrim(wktwriter_, 1);  // redundant, but keep to ensure it stays trim
    GEOSWKTWriter_setOutputDimension(wktwriter_, 3);
    ensure_equals("getOutputDimension_3", GEOSWKTWriter_getOutputDimension(wktwriter_), 3);

    test_writer_wkt("POINT (10 13)");
    test_writer_wkt("POINT Z (10 13 3)");
    test_writer_wkt("POINT M (10 13 3)");
    test_writer_wkt("POINT ZM (10 13 3 5)", "POINT Z (10 13 3)");
}

// Check Old3D with output dimension 3
template<>
template<>
void object::test<4>()
{
    GEOSWKTWriter_setOld3D(wktwriter_, 1);
    GEOSWKTWriter_setOutputDimension(wktwriter_, 3);

    test_writer_wkt("POINT (10 13)");
    test_writer_wkt("POINT (10 13 3)");
    test_writer_wkt("POINT M (10 13 5)");
    test_writer_wkt("POINT ZM (10 13 3 5)", "POINT (10 13 3)");

}

// Check Old3D with default output dimension 4
template<>
template<>
void object::test<5>()
{
    GEOSWKTWriter_setOld3D(wktwriter_, 1);
    ensure_equals(GEOSWKTWriter_getOutputDimension(wktwriter_), 4);

    test_writer_wkt("POINT (10 13)");
    test_writer_wkt("POINT (10 13 3)");
    test_writer_wkt("POINT M (10 13 5)");
    test_writer_wkt("POINT (10 13 3 5)");

}

// Check to get legacy default output before GEOS 3.12, showing untrimmed 2D WKT
template<>
template<>
void object::test<6>()
{
    GEOSWKTWriter_setTrim(wktwriter_, 0);
    GEOSWKTWriter_setOutputDimension(wktwriter_, 2);

    std::string expected("POINT (10.0000000000000000 13.0000000000000000)");

    test_writer_wkt("POINT (10 13)", expected);
    test_writer_wkt("POINT Z (10 13 3)", expected);
    test_writer_wkt("POINT M (10 13 5)", expected);
    test_writer_wkt("POINT ZM (10 13 3 5)", expected);

}

// Check untrimmed WKT with a fixed precision
template<>
template<>
void object::test<7>()
{
    GEOSWKTWriter_setTrim(wktwriter_, 0);
    GEOSWKTWriter_setRoundingPrecision(wktwriter_, 2);

    test_writer_wkt("POINT (10 13)", "POINT (10.00 13.00)");
    test_writer_wkt("POINT Z (10 13 3)", "POINT Z (10.00 13.00 3.00)");
    test_writer_wkt("POINT M (10 13 5)", "POINT M (10.00 13.00 5.00)");
    test_writer_wkt("POINT ZM (10 13 3 5)", "POINT ZM (10.00 13.00 3.00 5.00)");

}

} // namespace tut

