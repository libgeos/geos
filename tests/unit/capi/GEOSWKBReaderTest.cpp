#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cctype>
#include <cstring>
#include <string>
#include <utility>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoswkbreader_data : public capitest::utility {

    test_geoswkbreader_data() :
        wkbreader_(GEOSWKBReader_create())
    {}

    ~test_geoswkbreader_data() {
        GEOSWKBReader_destroy(wkbreader_);
    }

    GEOSWKBReader* wkbreader_;
};

typedef test_group<test_geoswkbreader_data> group;
typedef group::object object;

group test_geoswkbreader("capi::GEOSWKBReader");

// Test readHEX
template<>
template<>
void object::test<1>()
{
    std::string wkb = "01010000a0917d0000000000000000084000000000000020400000000000000000";

    geom1_ = GEOSWKBReader_readHEX(wkbreader_, (unsigned char*) wkb.c_str(), wkb.size());

    ensure_equals(GEOSGeomTypeId(geom1_), GEOS_POINT);
    ensure_equals(GEOSGetSRID(geom1_), 32145);

    double x = -1;
    double y = -1;
    double z = -1;

    GEOSGeomGetX(geom1_, &x);
    GEOSGeomGetY(geom1_, &y);
    GEOSGeomGetZ(geom1_, &z);

    ensure_equals(x, 3);
    ensure_equals(y, 8);
    ensure_equals(z, 0);
}

// Test fixStructure
template<>
template<>
void object::test<2>()
{
    // WKB corresponding to POLYGON ((0 0, 1 0, 1 1, 0 1)) <-- not closed
    std::string wkb = "0103000000010000000400000000000000000000000000000000000000000000000000f03f0000000000000000000000000000f03f000000000000f03f0000000000000000000000000000f03f";

    geom1_ = GEOSWKBReader_readHEX(wkbreader_, (unsigned char*) wkb.c_str(), wkb.size());
    ensure(geom1_ == nullptr);

    GEOSWKBReader_setFixStructure(wkbreader_, 1);
    geom1_ = GEOSWKBReader_readHEX(wkbreader_, (unsigned char*) wkb.c_str(), wkb.size());

    expected_ = fromWKT("POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))");
    ensure_geometry_equals(geom1_, expected_);
}

// readHEX and GEOSGeomFromHEX_buf accept upper, lower and mixed case
template<>
template<>
void object::test<3>()
{
    expected_ = fromWKT("POINT (0.1 -2.75)");
    const char* hexes[] = {
        "01010000009A9999999999B93F00000000000006C0",
        "01010000009a9999999999b93f00000000000006c0",
        "01010000009a9999999999B93f00000000000006C0",
        "00000000013FB999999999999AC006000000000000",
        "00000000013fb999999999999ac006000000000000"
    };

    for(const char* hex : hexes) {
        const unsigned char* buf = reinterpret_cast<const unsigned char*>(hex);
        geom1_ = GEOSWKBReader_readHEX(wkbreader_, buf, std::strlen(hex));
        geom2_ = GEOSGeomFromHEX_buf(buf, std::strlen(hex));
        ensure_geometry_equals_identical(geom1_, expected_);
        ensure_geometry_equals_identical(geom2_, expected_);
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        geom1_ = geom2_ = nullptr;
    }
}

// readHEX and GEOSGeomFromHEX_buf error messages
template<>
template<>
void object::test<4>()
{
    GEOSContextHandle_t context = GEOS_init_r();
    std::string errorMsg;
    GEOSContext_setErrorMessageHandler_r(context, [](const char* message, void* userdata) {
        *static_cast<std::string*>(userdata) = message;
    }, &errorMsg);
    GEOSWKBReader* reader = GEOSWKBReader_create_r(context);

    const std::pair<std::string, std::string> cases[] = {
        {"", "ParseException: Unexpected EOF parsing WKB"},
        {"0101000000", "ParseException: Input buffer is smaller than requested object size"},
        {"0", "ParseException: Premature end of HEX string"},
        {"01010000009A9999999999B93F00000000000006C", "ParseException: Premature end of HEX string"},
        {"01G", "ParseException: Premature end of HEX string"},
        {"0G1", "ParseException: Invalid HEX char"},
        {"0101000000 9A9999999999B93F00000000000006C0", "ParseException: Invalid HEX char"},
        {std::string("01\0" "1", 4), "ParseException: Invalid HEX char"},
        {"\xC3\xA9", "ParseException: Invalid HEX char"}
    };

    for(const auto& c : cases) {
        const unsigned char* buf = reinterpret_cast<const unsigned char*>(c.first.data());

        errorMsg.clear();
        ensure(GEOSWKBReader_readHEX_r(context, reader, buf, c.first.size()) == nullptr);
        ensure_equals(errorMsg, c.second);

        errorMsg.clear();
        ensure(GEOSGeomFromHEX_buf_r(context, buf, c.first.size()) == nullptr);
        ensure_equals(errorMsg, c.second);
    }

    GEOSWKBReader_destroy_r(context, reader);
    GEOS_finish_r(context);
}

// Hex round trip of a large geometry in both byte orders
template<>
template<>
void object::test<5>()
{
    std::string wkt = "LINESTRING (";
    for(int i = 0; i < 1000; ++i) {
        if(i > 0) {
            wkt += ", ";
        }
        wkt += std::to_string(i) + ".125 " + std::to_string(1000 - i) + ".5";
    }
    wkt += ")";
    input_ = fromWKT(wkt.c_str());

    static const char digits[] = "0123456789ABCDEF";
    GEOSWKBWriter* writer = GEOSWKBWriter_create();
    const int byteOrders[] = {GEOS_WKB_XDR, GEOS_WKB_NDR};

    for(int byteOrder : byteOrders) {
        GEOSWKBWriter_setByteOrder(writer, byteOrder);

        std::size_t wkbSize = 0;
        unsigned char* wkb = GEOSWKBWriter_write(writer, input_, &wkbSize);
        std::string expected;
        for(std::size_t i = 0; i < wkbSize; ++i) {
            expected += digits[wkb[i] >> 4];
            expected += digits[wkb[i] & 0x0F];
        }
        GEOSFree(wkb);

        std::size_t hexSize = 0;
        unsigned char* hex = GEOSWKBWriter_writeHEX(writer, input_, &hexSize);
        ensure_equals(std::string(reinterpret_cast<const char*>(hex), hexSize), expected);

        std::string lower = expected;
        for(char& ch : lower) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }

        geom1_ = GEOSWKBReader_readHEX(wkbreader_, hex, hexSize);
        geom2_ = GEOSGeomFromHEX_buf(reinterpret_cast<const unsigned char*>(lower.data()), lower.size());
        ensure_geometry_equals_identical(geom1_, input_);
        ensure_geometry_equals_identical(geom2_, input_);
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        geom1_ = geom2_ = nullptr;
        GEOSFree(hex);
    }

    GEOSWKBWriter_destroy(writer);

    std::size_t size = 0;
    unsigned char* hex = GEOSGeomToHEX_buf(input_, &size);
    geom1_ = GEOSGeomFromHEX_buf(hex, size);
    ensure_geometry_equals_identical(geom1_, input_);
    GEOSFree(hex);
}

} // namespace tut

