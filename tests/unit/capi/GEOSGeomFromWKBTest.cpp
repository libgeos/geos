//
// Test Suite for C-API GEOSGeomFromWKB

#include <tut/tut.hpp>
#include <utility.h> // wkb_hex_decoder

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosgeomfromwkb_data : public capitest::utility {
    GEOSWKTReader* reader_;

    test_capigeosgeomfromwkb_data() : reader_(nullptr)
    {
        reader_ = GEOSWKTReader_create();
    }

    ~test_capigeosgeomfromwkb_data()
    {
        GEOSWKTReader_destroy(reader_);
        reader_ = nullptr;
    }

    void
    test_wkb(const std::string& wkbhex, const std::string& wkt)
    {
        wkb_hex_decoder::binary_type wkb;
        wkb_hex_decoder::decode(wkbhex, wkb);

        geom1_ = GEOSGeomFromWKB_buf(&wkb[0], wkb.size());
        ensure("GEOSGeomFromWKB_buf failed to create geometry", nullptr != geom1_);

        geom2_ = GEOSWKTReader_read(reader_, wkt.c_str());
        ensure ("GEOSWKTReader_read failed to create geometry", nullptr != geom2_ );
        ensure_geometry_equals(geom1_, geom2_);
    }
};

typedef test_group<test_capigeosgeomfromwkb_data> group;
typedef group::object object;

group test_capigeosgeomfromwkb_group("capi::GEOSGeomFromWKB");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    // POINT(1.234 5.678)
    std::string wkt("POINT (1.234 5.678)");
    std::string wkb("01010000005839B4C876BEF33F83C0CAA145B61640");
    test_wkb(wkb, wkt);
}

template<>
template<>
void object::test<2>
()
{
    // SRID=4;POINT(0 0)
    std::string wkt("POINT(0 0)");
    std::string ewkb("01010000200400000000000000000000000000000000000000");
    test_wkb(ewkb, wkt);
}

template<>
template<>
void object::test<3>
()
{
    // SRID=32632;POINT(1.234 5.678)
    std::string wkt("POINT (1.234 5.678)");
    std::string ewkb("0101000020787F00005839B4C876BEF33F83C0CAA145B61640");
    test_wkb(ewkb, wkt);
}

template<>
template<>
void object::test<4>
()
{
    // POINT (1.234 5.678 15 79) -- XYZM
    std::string wkt("POINT (1.234 5.678 15 79)");
    std::string ewkb("01010000C05839B4C876BEF33F83C0CAA145B616400000000000002E400000000000C05340");
    test_wkb(ewkb, wkt);
}

template<>
template<>
void object::test<5>
()
{
    std::string wkt("MULTIPOINT ((1.123 1.456), (2.123 2.456), (3.123 3.456))");
    std::string
    ewkb("01040000000300000001010000002b8716d9cef7f13fb29defa7c64bf73f010100000096438b6ce7fb0040d9cef753e3a50340010100000096438b6ce7fb0840d9cef753e3a50b40");
    test_wkb(ewkb, wkt);
}

// Check force close on unclosed rings
template<>
template<>
void object::test<6>
()
{
    geom1_ = GEOSWKTReader_read(reader_, "POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))");
    ensure("geom1_ not null", geom1_ != nullptr);
    GEOSWKTReader_setFixStructure(reader_, 1);
    geom2_ = GEOSWKTReader_read(reader_, "POLYGON((0 0, 0 1, 1 1, 1 0))");
    ensure("geom2_ not null", geom2_ != nullptr);
    ensure_geometry_equals(geom1_, geom2_);
}

// Supply EWKB input
template<>
template<>
void object::test<7>()
{
    test_wkb(
        "01040000C00200000001010000C000000000000000000000000000000000000000000000F03F000000000000F03F01010000C0000000000000084000000000000000400000000000000040000000000000F03F",
        "MULTIPOINT((0 0 1 1), (3 2 2 1))"
    );
}

} // namespace tut

