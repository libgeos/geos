/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/io/GeoJSONReader.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/PrecisionModel.h>

#include <algorithm>
#include <ostream>
#include <sstream>
#include <cassert>

#include <json.hpp>

#undef DEBUG_GEOJSON_READER

using namespace geos::geom;
using json = nlohmann::json;

namespace geos {
namespace io { // geos.io

GeoJSONReader::GeoJSONReader(): GeoJSONReader(*(GeometryFactory::getDefaultInstance())) {}

GeoJSONReader::GeoJSONReader(const geom::GeometryFactory& gf) : geometryFactory(gf) {}

std::unique_ptr<geom::Geometry> GeoJSONReader::read(const std::string& geoJsonText) {
    json j = json::parse(geoJsonText);
    std::string type = j["type"];
    if (type == "Feature") {
        return readFeatureForGeometry(j);    
    } else if (type == "FeatureCollection") {
        return readFeatureCollectionForGeometry(j);    
    } else {
        return readGeometry(j);
    }
}

GeoJSONFeatureCollection GeoJSONReader::readFeatures(const std::string& geoJsonText) {
    json j = json::parse(geoJsonText);
    std::string type = j["type"];
    if (type == "Feature") {
        auto feature = readFeature(j);    
        return GeoJSONFeatureCollection { std::vector<GeoJSONFeature>{feature} };
    } else if (type == "FeatureCollection") {
        return readFeatureCollection(j);    
    } else {
        auto g = readGeometry(j);
        return GeoJSONFeatureCollection { std::vector<GeoJSONFeature>{GeoJSONFeature{std::move(g), std::map<std::string, GeoJSONValue>{} }}};
    }
}

std::unique_ptr<geom::Geometry> GeoJSONReader::readFeatureForGeometry(nlohmann::json& j) {
    auto geometryJson = j["geometry"];
    auto geometry = readGeometry(geometryJson);
    return geometry;
}

GeoJSONFeature GeoJSONReader::readFeature(nlohmann::json& j) {
    auto geometryJson = j["geometry"];
    auto geometry = readGeometry(geometryJson);
    auto properties = j["properties"];
    std::map<std::string,GeoJSONValue> map = readProperties(properties);    
    GeoJSONFeature f = GeoJSONFeature{ std::move(geometry), std::move(map) };
    return f;
}

std::map<std::string,GeoJSONValue> GeoJSONReader::readProperties(nlohmann::json& p) {
    std::map<std::string,GeoJSONValue> map;
    for(auto prop : p.items()) {
        map[prop.key()] = std::move(readProperty(prop.value()));
    }
    return map;
}

GeoJSONValue GeoJSONReader::readProperty(nlohmann::json& value) {
    if (value.is_string()) {
        return GeoJSONValue { value.get<std::string>() };
    } else if (value.is_number()) {
        return GeoJSONValue { value.get<double>() };
    } else if (value.is_boolean()) {
        return GeoJSONValue { value.get<bool>() };
    } else if (value.is_array()) {
        std::vector<GeoJSONValue> v {};
        for (auto& el : value.items()) {
            const GeoJSONValue item = readProperty(el.value());
            v.push_back(item);
        }
        return GeoJSONValue{ v };
    } else if (value.is_object()) {
        std::map<std::string, GeoJSONValue> v {};
        for (auto& el : value.items()) {
            v[el.key()] = readProperty(el.value());
        }
        return GeoJSONValue{ v };
    } else {
        return GeoJSONValue{};
    }
}

std::unique_ptr<geom::Geometry> GeoJSONReader::readFeatureCollectionForGeometry(nlohmann::json& j) {
    auto featuresJson = j["features"];
    std::vector<geom::Geometry *>* geometries = new std::vector<geom::Geometry *>();
    for(auto featureJson : featuresJson) {
        auto g = readFeatureForGeometry(featureJson);
        geometries->push_back(g.release());
    }
    return std::unique_ptr<geom::GeometryCollection>(geometryFactory.createGeometryCollection(geometries));
}

GeoJSONFeatureCollection GeoJSONReader::readFeatureCollection(nlohmann::json& j) {
    auto featuresJson = j["features"];
    std::vector<GeoJSONFeature> features;
    for(auto featureJson : featuresJson) {
        auto f = readFeature(featureJson);
        features.push_back(f);
    }
    return GeoJSONFeatureCollection{features};
}


std::unique_ptr<geom::Geometry> GeoJSONReader::readGeometry(nlohmann::json& j) {
    std::string type = j["type"];
    if (type == "Point") {
        return readPoint(j);
    } else if (type == "LineString") {
        return readLineString(j);
    } else if (type == "Polygon") {
        return readPolygon(j);
    } else if (type == "MultiPoint") {
        return readMultiPoint(j);
    } else if (type == "MultiLineString") {
        return readMultiLineString(j);
    } else if (type == "MultiPolygon") {
        return readMultiPolygon(j);
    } else if (type == "GeometryCollection") {
        return readGeometryCollection(j);
    }
    return std::unique_ptr<geom::Geometry>(geometryFactory.createEmptyGeometry());
}

std::unique_ptr<geom::Point> GeoJSONReader::readPoint(nlohmann::json& j) {
    std::vector<double> coords = j["coordinates"].get<std::vector<double>>();
    if (coords.size() == 0) {
        return std::unique_ptr<geom::Point>(geometryFactory.createPoint(2));
    } else {
        geom::Coordinate coord = {coords[0], coords[1]};
        return std::unique_ptr<geom::Point>(geometryFactory.createPoint(coord));
    }
}

std::unique_ptr<geom::LineString> GeoJSONReader::readLineString(nlohmann::json& j) {
    std::vector<std::pair<double,double>> coords = j["coordinates"].get<std::vector<std::pair<double,double>>>();
    std::vector<geom::Coordinate> coordinates;
    for(std::vector<int>::size_type i = 0; i < coords.size(); i++) {
        coordinates.push_back(geom::Coordinate{coords[i].first, coords[i].second});
    }
    geom::CoordinateArraySequence coordinateSequence { std::move(coordinates) };
    return std::unique_ptr<geom::LineString>(geometryFactory.createLineString(coordinateSequence));
}

std::unique_ptr<geom::Polygon> GeoJSONReader::readPolygon(nlohmann::json& json) {
    std::vector<std::vector<std::pair<double,double>>> polygonCoords = json["coordinates"].get<std::vector<std::vector<std::pair<double,double>>>>();
    std::vector<geom::LinearRing *> rings;
    for(std::vector<int>::size_type i = 0; i < polygonCoords.size(); i++) {
        std::vector<geom::Coordinate> coordinates;
        for (std::vector<int>::size_type j = 0; j < polygonCoords[i].size(); j++) {
            coordinates.push_back(geom::Coordinate{polygonCoords[i][j].first, polygonCoords[i][j].second});
        }
        geom::CoordinateArraySequence coordinateSequence { std::move(coordinates) };
        rings.push_back(geometryFactory.createLinearRing(std::move(coordinateSequence)));
    }
    if (rings.size() == 0) {
        return std::unique_ptr<geom::Polygon>(geometryFactory.createPolygon(2));
    } else if (rings.size() == 1) {
        geom::LinearRing* outerRing = rings[0];
        std::vector<geom::LinearRing *>* innerRings {};
        return std::unique_ptr<geom::Polygon>(geometryFactory.createPolygon(outerRing, innerRings));
    } else {
        geom::LinearRing* outerRing = rings[0];
        std::vector<geom::LinearRing *>* innerRings = new std::vector<geom::LinearRing *>(rings.begin() + 1, rings.end());
        return std::unique_ptr<geom::Polygon>(geometryFactory.createPolygon(outerRing, innerRings));        
    }
}

std::unique_ptr<geom::MultiPoint> GeoJSONReader::readMultiPoint(nlohmann::json& j) {
    std::vector<std::pair<double,double>> coords = j["coordinates"].get<std::vector<std::pair<double,double>>>();
    std::vector<geom::Coordinate> coordinates;
    for(std::vector<int>::size_type i = 0; i < coords.size(); i++) {
        coordinates.push_back(geom::Coordinate{coords[i].first, coords[i].second});
    }
    geom::CoordinateArraySequence coordinateSequence { std::move(coordinates) };
    return std::unique_ptr<geom::MultiPoint>(geometryFactory.createMultiPoint(coordinateSequence));
}

std::unique_ptr<geom::MultiLineString> GeoJSONReader::readMultiLineString(nlohmann::json& json) {
    std::vector<std::vector<std::pair<double,double>>> coords = json["coordinates"].get<std::vector<std::vector<std::pair<double,double>>>>();
    std::vector<geom::Geometry *>* lines = new std::vector<geom::Geometry *>{};
    for(std::vector<int>::size_type i = 0; i < coords.size(); i++) {
        std::vector<geom::Coordinate> coordinates;
        for (std::vector<int>::size_type j = 0; j < coords[i].size(); j++) {
            coordinates.push_back(geom::Coordinate{coords[i][j].first, coords[i][j].second});
        }
        geom::CoordinateArraySequence coordinateSequence { std::move(coordinates) };
        lines->push_back(geometryFactory.createLineString(std::move(coordinateSequence)));
    }
    return std::unique_ptr<geom::MultiLineString>(geometryFactory.createMultiLineString(lines));
}

std::unique_ptr<geom::MultiPolygon> GeoJSONReader::readMultiPolygon(nlohmann::json& json) {
    std::vector<std::vector<std::vector<std::pair<double,double>>>> multiPolygonCoords = json["coordinates"].get<std::vector<std::vector<std::vector<std::pair<double,double>>>>>();
    std::vector<geom::Geometry *>* polygons = new std::vector<geom::Geometry *>();
    for(std::vector<int>::size_type i = 0; i < multiPolygonCoords.size(); i++) {
        std::vector<std::vector<std::pair<double,double>>> polygonCoords = multiPolygonCoords[i];
        std::vector<geom::LinearRing *> rings;
        for(std::vector<int>::size_type j = 0; j < polygonCoords.size(); j++) {
            std::vector<geom::Coordinate> coordinates;
            for (std::vector<int>::size_type k = 0; k < polygonCoords[j].size(); k++) {
                coordinates.push_back(geom::Coordinate{polygonCoords[j][k].first, polygonCoords[j][k].second});
            }
            geom::CoordinateArraySequence coordinateSequence { std::move(coordinates) };
            rings.push_back(geometryFactory.createLinearRing(std::move(coordinateSequence)));
        }
        if (rings.size() == 1) {
            geom::LinearRing* outerRing = rings[0];
            std::vector<geom::LinearRing *>* innerRings {};
            polygons->push_back(geometryFactory.createPolygon(outerRing, innerRings));
        } else {
            geom::LinearRing* outerRing = rings[0];
            std::vector<geom::LinearRing *>* innerRings = new std::vector<geom::LinearRing *>(rings.begin() + 1, rings.end());
            polygons->push_back(geometryFactory.createPolygon(outerRing, innerRings));        
        }
    }
    return std::unique_ptr<geom::MultiPolygon>(geometryFactory.createMultiPolygon(polygons));
}

std::unique_ptr<geom::GeometryCollection> GeoJSONReader::readGeometryCollection(nlohmann::json& j) {
    std::vector<geom::Geometry *>* geometries = new std::vector<geom::Geometry *>();
    auto jsonGeometries = j["geometries"];
    for (auto jsonGeometry : jsonGeometries) {
        auto g = readGeometry(jsonGeometry);
        geometries->push_back(g.release());
    }
    return std::unique_ptr<geom::GeometryCollection>(geometryFactory.createGeometryCollection(geometries));
}

} // namespace geos.io
} // namespace geos