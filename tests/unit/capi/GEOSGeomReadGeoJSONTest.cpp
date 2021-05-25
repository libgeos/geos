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
    GEOSWKTReader* reader_;

    test_capigeosgeomreadgeojson_data() : reader_(nullptr)
    {
        reader_ = GEOSGeoJSONReader_create();
    }

    ~test_capigeosgeomreadgeojson_data()
    {
        GEOSGeoJSONReader_destroy(reader_);
        reader_ = nullptr;
    }

    void
    test_geojson(std::string const& geojson, std::string const& wkt)
    {
        geom1_ = GEOSGeoJSONReader_read(&geojson[0]);
        // TODO: Update test to compare with WKT-based geometry
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
    std::string geojson('{"type":"FeatureCollection","features":[{"type":"Feature","properties":{},"geometry":{"type":"Point","coordinates":[5.05,48.37]}}]');
    std::string wkt("GEOMETRYCOLLECTION(POINT(5.05 48.37))");
    test_geojson(geojson, wkt);
}
