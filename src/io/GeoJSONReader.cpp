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
#include <geos/io/ParseException.h>
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
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/PrecisionModel.h>

#include <algorithm>
#include <ostream>
#include <sstream>
#include <cassert>

#include <json.hpp>

using namespace geos::geom;
using json = nlohmann::json;

namespace geos {
namespace io { // geos.io

GeoJSONReader::GeoJSONReader(): GeoJSONReader(*(GeometryFactory::getDefaultInstance())) {}

GeoJSONReader::GeoJSONReader(const geom::GeometryFactory& gf) : geometryFactory(gf) {}

std::unique_ptr<geom::Geometry> GeoJSONReader::read(const std::string& geoJsonText) {
    try {
        json j = json::parse(geoJsonText);
        std::string type = j["type"];
        if (type == "Feature") {
            return readFeatureForGeometry(j);    
        } else if (type == "FeatureCollection") {
            return readFeatureCollectionForGeometry(j);    
        } else {
            return readGeometry(j);
        }
    } catch (json::exception& ex) {
        throw ParseException("Error parsing JSON");
    }   
}

GeoJSONFeatureCollection GeoJSONReader::readFeatures(const std::string& geoJsonText) {
    try {
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
    } catch (json::exception& ex) {
        throw ParseException("Error parsing JSON");
    }
}

std::unique_ptr<geom::Geometry> GeoJSONReader::readFeatureForGeometry(const nlohmann::json& j) {
    auto geometryJson = j["geometry"];
    auto geometry = readGeometry(geometryJson);
    return geometry;
}

GeoJSONFeature GeoJSONReader::readFeature(const nlohmann::json& j) {
    auto geometryJson = j["geometry"];
    auto geometry = readGeometry(geometryJson);
    auto properties = j["properties"];
    std::map<std::string,GeoJSONValue> map = readProperties(properties);    
    GeoJSONFeature f = GeoJSONFeature{ std::move(geometry), std::move(map) };
    return f;
}

std::map<std::string,GeoJSONValue> GeoJSONReader::readProperties(const nlohmann::json& p) {
    std::map<std::string,GeoJSONValue> map;
    for(const auto& prop : p.items()) {
        map[prop.key()] = std::move(readProperty(prop.value()));
    }
    return map;
}

GeoJSONValue GeoJSONReader::readProperty(const nlohmann::json& value) {
    if (value.is_string()) {
        return GeoJSONValue { value.get<std::string>() };
    } else if (value.is_number()) {
        return GeoJSONValue { value.get<double>() };
    } else if (value.is_boolean()) {
        return GeoJSONValue { value.get<bool>() };
    } else if (value.is_array()) {
        std::vector<GeoJSONValue> v {};
        for (const auto& el : value.items()) {
            const GeoJSONValue item = readProperty(el.value());
            v.push_back(item);
        }
        return GeoJSONValue{ v };
    } else if (value.is_object()) {
        std::map<std::string, GeoJSONValue> v {};
        for (const auto& el : value.items()) {
            v[el.key()] = readProperty(el.value());
        }
        return GeoJSONValue{ v };
    } else {
        return GeoJSONValue{};
    }
}

std::unique_ptr<geom::Geometry> GeoJSONReader::readFeatureCollectionForGeometry(const nlohmann::json& j) {
    auto featuresJson = j["features"];
    std::vector<std::unique_ptr<geom::Geometry>> geometries;
    for(const auto& featureJson : featuresJson) {
        auto g = readFeatureForGeometry(featureJson);
        geometries.push_back(std::move(g));
    }
    return geometryFactory.createGeometryCollection(std::move(geometries));
}

GeoJSONFeatureCollection GeoJSONReader::readFeatureCollection(const nlohmann::json& j) {
    auto featuresJson = j["features"];
    std::vector<GeoJSONFeature> features;
    for(const auto& featureJson : featuresJson) {
        auto f = readFeature(featureJson);
        features.push_back(f);
    }
    return GeoJSONFeatureCollection{features};
}


std::unique_ptr<geom::Geometry> GeoJSONReader::readGeometry(const nlohmann::json& j) {
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
    } else {
        throw ParseException{"Unknown geometry type!"};
    }
}

geom::Coordinate GeoJSONReader::readCoordinate(const std::vector<double>& coords) {
    if (coords.size() == 1) {
        throw  ParseException("Expected two coordinates found one");
    } else {
        return geom::Coordinate {coords[0], coords[1]};
    }
}

std::unique_ptr<geom::Point> GeoJSONReader::readPoint(const nlohmann::json& j) {
    auto coords = j["coordinates"].get<std::vector<double>>();
    if (coords.size() == 1) {
        throw  ParseException("Expected two coordinates found one");
    } else if (coords.size() < 2) {
        return geometryFactory.createPoint(2);
    } else {
        geom::Coordinate coord = readCoordinate(coords);
        return std::unique_ptr<geom::Point>(geometryFactory.createPoint(coord));
    }
}

std::unique_ptr<geom::LineString> GeoJSONReader::readLineString(const nlohmann::json& j) {
    auto coords = j["coordinates"].get<std::vector<std::vector<double>>>();
    std::vector<geom::Coordinate> coordinates;
    for(const auto& coord : coords) {
        geom::Coordinate c = readCoordinate(coord);
        coordinates.push_back(geom::Coordinate{c.x,c.y});
    }
    auto coordinateSequence = geometryFactory.getCoordinateSequenceFactory()->create(std::move(coordinates));
    return geometryFactory.createLineString(std::move(coordinateSequence));
}

std::unique_ptr<geom::Polygon> GeoJSONReader::readPolygon(const nlohmann::json& json) {
    auto polygonCoords = json["coordinates"].get<std::vector<std::vector<std::vector<double>>>>();
    return readPolygon(polygonCoords);
}

std::unique_ptr<geom::Polygon> GeoJSONReader::readPolygon(const std::vector<std::vector<std::vector<double>>>& polygonCoords) {
    std::unique_ptr<geom::LinearRing> shell;
    std::vector<std::unique_ptr<geom::LinearRing>> rings;
    for(const auto& ring : polygonCoords) {
        std::vector<geom::Coordinate> coordinates;
        for (const auto& coord : ring) {
            geom::Coordinate c = readCoordinate(coord);
            coordinates.push_back(geom::Coordinate{c.x, c.y});
        }
        auto coordinateSequence = geometryFactory.getCoordinateSequenceFactory()->create(std::move(coordinates));
        if (!shell) {
            shell = geometryFactory.createLinearRing(std::move(coordinateSequence));
        } else {
            rings.push_back(geometryFactory.createLinearRing(std::move(coordinateSequence)));
        }
    }
    if (!shell) {
        return geometryFactory.createPolygon(2);
    } else if (rings.size() == 0) {
        return geometryFactory.createPolygon(std::move(shell));
    } else {
        return geometryFactory.createPolygon(std::move(shell), std::move(rings));        
    }
}

std::unique_ptr<geom::MultiPoint> GeoJSONReader::readMultiPoint(const nlohmann::json& j) {
    auto coords = j["coordinates"].get<std::vector<std::vector<double>>>();
    std::vector<std::unique_ptr<geom::Point>> points;
    for(const auto& coord : coords) {
        geom::Coordinate c = readCoordinate(coord);
        points.push_back(std::unique_ptr<geom::Point>(geometryFactory.createPoint(c)));
    }
    return geometryFactory.createMultiPoint(std::move(points));
}

std::unique_ptr<geom::MultiLineString> GeoJSONReader::readMultiLineString(const nlohmann::json& json) {
    auto listOfCoords = json["coordinates"].get<std::vector<std::vector<std::vector<double>>>>();
    std::vector<std::unique_ptr<geom::LineString>> lines;
    for(const auto& coords :  listOfCoords) {    
        std::vector<geom::Coordinate> coordinates;
        for (const auto& coord : coords) {
            geom::Coordinate c = readCoordinate(coord);
            coordinates.push_back(geom::Coordinate{c.x, c.y});
        }
        auto coordinateSequence = geometryFactory.getCoordinateSequenceFactory()->create(std::move(coordinates));
        lines.push_back(geometryFactory.createLineString(std::move(coordinateSequence)));
    }
    return geometryFactory.createMultiLineString(std::move(lines));
}

std::unique_ptr<geom::MultiPolygon> GeoJSONReader::readMultiPolygon(const nlohmann::json& json) {
    auto multiPolygonCoords = json["coordinates"].get<std::vector<std::vector<std::vector<std::vector<double>>>>>();
    std::vector<std::unique_ptr<geom::Polygon>> polygons;
    for(const auto& polygonCoords : multiPolygonCoords) {    
        polygons.push_back(readPolygon(polygonCoords));
    }
    return geometryFactory.createMultiPolygon(std::move(polygons));
}

std::unique_ptr<geom::GeometryCollection> GeoJSONReader::readGeometryCollection(const nlohmann::json& j) {
    std::vector<std::unique_ptr<geom::Geometry>> geometries;
    auto jsonGeometries = j["geometries"];
    for (const auto& jsonGeometry : jsonGeometries) {
        auto g = readGeometry(jsonGeometry);
        geometries.push_back(std::move(g));
    }
    return geometryFactory.createGeometryCollection(std::move(geometries));
}

} // namespace geos.io
} // namespace geos