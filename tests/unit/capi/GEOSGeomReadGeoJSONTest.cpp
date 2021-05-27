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
        geom1_ = GEOSGeoJSONReader_read(reader_, &geojson[0]);
        ensure("GEOSGeoJSONReader_read failed to create geometry", nullptr != geom1_);
        ensure_geometry_equals(geom1_, fromWKT(&wkt[0]));
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
    std::string geojson("{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}");
    std::string wkt("POINT(-117.0 33.0)");
    test_geojson(geojson, wkt);
}

template<>
template<>
void object::test<2>
()
{
    std::string geojson("{\"type\":\"FeatureCollection\",\"features\":[{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}},{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-122.0,45.0]}}]}");
    std::string wkt("GEOMETRYCOLLECTION (POINT (-117.000 33.000), POINT (-122.000 45.000))");
    test_geojson(geojson, wkt);
}

template<>
template<>
void object::test<3>
()
{
    std::string geojson("<gml>NOT_GEO_JSON</gml>");
    geom1_ = GEOSGeoJSONReader_read(reader_, &geojson[0]);
    ensure(geom1_ == nullptr);
}
}
