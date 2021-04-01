//
// Test Suite for geos::io::GeoJSONFeture and geos::io::GeoJSONFetureCollection

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/GeoJSON.h>
#include <geos/io/WKTReader.h>
#include <geos/io/GeoJSONWriter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {

//
// Test Group
//
struct test_geojson_feature_data {
    geos::geom::PrecisionModel pm;
    geos::geom::GeometryFactory::Ptr gf;
    geos::io::WKTReader wktreader;

    test_geojson_feature_data()
        :
        pm(1000.0),
        gf(geos::geom::GeometryFactory::create(&pm)),
        wktreader(gf.get())
    {}
};

typedef test_group<test_geojson_feature_data> group;
typedef group::object object;

group test_geojson_feature_data("geos::io::GeoJSONFeature");

// Create a GeoJSON Feature
template<>
template<>
void object::test<1>
()
{
    geos::io::GeoJSONFeature feature { wktreader.read("POINT(-117 33)"), std::map<std::string, geos::io::GeoJSONValue> {
        {"id",   geos::io::GeoJSONValue(1.0)     },
        {"name", geos::io::GeoJSONValue(std::string{"One"}) },
    }};
    ensure_equals("POINT (-117.000 33.000)", feature.getGeometry()->toText());
    ensure_equals(1.0, feature.getProperties().at("id").getNumber());
    ensure_equals("One", feature.getProperties().at("name").getString());

    const geos::io::GeoJSONFeature feature2 = feature;
    ensure_equals("POINT (-117.000 33.000)", feature2.getGeometry()->toText());
    ensure_equals(1.0, feature2.getProperties().at("id").getNumber());
    ensure_equals("One", feature2.getProperties().at("name").getString());
}

// Write a GeoJSON FeatureCollection
template<>
template<>
void object::test<2>
()
{
    geos::io::GeoJSONFeatureCollection features {{
        geos::io::GeoJSONFeature { wktreader.read("POINT(-117 33)"), std::map<std::string, geos::io::GeoJSONValue> {
            {"id",   geos::io::GeoJSONValue(1.0)     },
            {"name", geos::io::GeoJSONValue(std::string{"One"}) },
        }},
        geos::io::GeoJSONFeature { wktreader.read("POINT(-127 53)"), std::map<std::string, geos::io::GeoJSONValue> {
            {"id",   geos::io::GeoJSONValue(2.0)     },
            {"name", geos::io::GeoJSONValue(std::string{"Two"}) },
        }}
    }};
    ensure_equals(static_cast<size_t>(2), features.getFeatures().size());
    ensure_equals("POINT (-117.000 33.000)", features.getFeatures()[0].getGeometry()->toText());
    ensure_equals(1.0, features.getFeatures()[0].getProperties().at("id").getNumber());
    ensure_equals("One", features.getFeatures()[0].getProperties().at("name").getString());
    ensure_equals("POINT (-127.000 53.000)", features.getFeatures()[1].getGeometry()->toText());
    ensure_equals(2.0, features.getFeatures()[1].getProperties().at("id").getNumber());
    ensure_equals("Two", features.getFeatures()[1].getProperties().at("name").getString());
}

}