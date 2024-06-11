#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/util/Machine.h> // for getMachineByteOrder

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoswkbwriter_data : public capitest::utility {

    test_geoswkbwriter_data() :
        wkbwriter_(GEOSWKBWriter_create()),
        buf_(nullptr)
    {
        GEOSWKBWriter_setByteOrder(wkbwriter_, GEOS_WKB_NDR);
    }

    ~test_geoswkbwriter_data() {
        GEOSWKBWriter_destroy(wkbwriter_);
        GEOSFree(buf_);
    }

    GEOSWKBWriter* wkbwriter_;
    unsigned char* buf_;
};

typedef test_group<test_geoswkbwriter_data> group;
typedef group::object object;

group test_geoswkbwriter("capi::GEOSWKBWriter");

// WKBWriter defaults
template<>
template<>
void object::test<1>()
{
    GEOSWKBWriter_setByteOrder(wkbwriter_, GEOS_WKB_XDR);
    ensure_equals(GEOSWKBWriter_getIncludeSRID(wkbwriter_), 0);
    ensure_equals(GEOSWKBWriter_getByteOrder(wkbwriter_), GEOS_WKB_XDR);
    ensure_equals(GEOSWKBWriter_getFlavor(wkbwriter_), GEOS_WKB_EXTENDED);
    ensure_equals(GEOSWKBWriter_getOutputDimension(wkbwriter_), 4);
}

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("POINT (3 8)");
    GEOSSetSRID(geom1_, 32145);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // SELECT encode(ST_AsBinary('POINT (3 8)'::geometry), 'hex');
    ensure_equals(hexstr, "010100000000000000000008400000000000002040");
}

template<>
template<>
void object::test<3>()
{
    geom1_ = fromWKT("POINT (3 8)");
    GEOSSetSRID(geom1_, 32145);

    GEOSWKBWriter_setIncludeSRID(wkbwriter_, 1);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // SELECT encode(ST_AsEWKB('SRID=32145;POINT (3 8)'::geometry), 'hex');
    ensure_equals(hexstr, "0101000020917D000000000000000008400000000000002040");
}

template<>
template<>
void object::test<4>()
{
    geom1_ = fromWKT("POINT Z (3 8 0)");
    GEOSSetSRID(geom1_, 32145);

    GEOSWKBWriter_setIncludeSRID(wkbwriter_, 1);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // SELECT encode(ST_AsEWKB('SRID=32145;POINT Z(3 8 0)'::geometry), 'hex');
    ensure_equals(hexstr, "01010000A0917D0000000000000000084000000000000020400000000000000000");
}

template<>
template<>
void object::test<5>()
{
    geom1_ = fromWKT("POINT (3 8)");
    GEOSSetSRID(geom1_, 32145);

    GEOSWKBWriter_setByteOrder(wkbwriter_, GEOS_WKB_XDR);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // SELECT encode(ST_AsBinary('POINT (3 8)'::geometry, 'XDR'), 'hex');
    ensure_equals(hexstr, "000000000140080000000000004020000000000000");
}

template<>
template<>
void object::test<6>()
{
    geom1_ = fromWKT("POINT M (3 8 2)");

    GEOSWKBWriter_setOutputDimension(wkbwriter_, 2);
    ensure_equals(GEOSWKBWriter_getOutputDimension(wkbwriter_), 2);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // POINT (3 8)
    ensure_equals(hexstr, "010100000000000000000008400000000000002040");
}

template<>
template<>
void object::test<7>()
{
    geom1_ = fromWKT("POINT M (3 8 2)");

    GEOSWKBWriter_setOutputDimension(wkbwriter_, 3);
    ensure_equals(GEOSWKBWriter_getOutputDimension(wkbwriter_), 3);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // POINT M (3 8 2)
    ensure_equals(hexstr, "0101000040000000000000084000000000000020400000000000000040");
}

template<>
template<>
void object::test<8>()
{
    geom1_ = fromWKT("POINT ZM (3 8 1 2)");

    GEOSWKBWriter_setOutputDimension(wkbwriter_, 2);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // POINT (3 8)
    ensure_equals(hexstr, "010100000000000000000008400000000000002040");
}

template<>
template<>
void object::test<9>()
{
    geom1_ = fromWKT("POINT ZM (3 8 1 2)");

    GEOSWKBWriter_setOutputDimension(wkbwriter_, 3);
    ensure_equals(GEOSWKBWriter_getOutputDimension(wkbwriter_), 3);

    std::size_t hex_size = 0;
    buf_ = GEOSWKBWriter_writeHEX(wkbwriter_, geom1_, &hex_size);

    std::string hexstr = std::string((const char*) buf_, (const char*) buf_ + hex_size);

    // POINT Z (3 8 1)
    ensure_equals(hexstr, "010100008000000000000008400000000000002040000000000000F03F");
}

} // namespace tut

