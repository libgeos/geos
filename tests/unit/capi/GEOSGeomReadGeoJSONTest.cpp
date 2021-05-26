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
struct test_capigeosgeomreadgeojson_data : public capitest::utility {
    GEOSGeoJSONReader* reader_;
    GEOSWKTReader* wkt_reader_;

    test_capigeosgeomreadgeojson_data() : reader_(nullptr), wkt_reader_(nullptr)
    {
        reader_ = GEOSGeoJSONReader_create();
        wkt_reader_ = GEOSWKTReader_create();
    }

    ~test_capigeosgeomreadgeojson_data()
    {
        GEOSGeoJSONReader_destroy(reader_);
        GEOSWKTReader_destroy(wkt_reader_);
        reader_ = nullptr;
        wkt_reader_ = nullptr;
    }

    void
    test_geojson(std::string const& geojson, std::string const& wkt)
    {
        geom1_ = GEOSGeoJSONReader_read(reader_, &geojson[0]);
        expected_ = GEOSWKTReader_read(wkt_reader_, &wkt[0]);
        ensure_geometry_equals(geom1_, expected_);
    }
};

typedef test_group<test_capigeosgeomreadgeojson_data> group;
typedef group::object object;

group test_capigeosgeomreadgeojson_group("capi::GEOSGeomReadGeoJSON");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    // POINT(1.234 5.678)
    std::string geojson(R"({"type":"FeatureCollection","features":[{"type":"Feature","properties":{},"geometry":{"type":"Point","coordinates":[1.234,5.678]}}])");
    std::string wkt("GEOMETRYCOLLECTION(POINT(1.234 5.678))");
    test_geojson(geojson, wkt);
}
}
