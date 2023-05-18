/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Jared Erickson
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
#include <geos/geom/PrecisionModel.h>
#include <geos/util.h>

#include <algorithm>
#include <ostream>
#include <sstream>
#include <cassert>

#define PROJ_COMPILATION

using namespace geos::geom;
using json = geos_nlohmann::json;

namespace geos {
namespace io { // geos.io

GeoJSONReader::GeoJSONReader(): GeoJSONReader(*(GeometryFactory::getDefaultInstance())) {}

GeoJSONReader::GeoJSONReader(const geom::GeometryFactory& gf) : geometryFactory(gf) {}

std::unique_ptr<geom::Geometry> GeoJSONReader::read(const std::string& geoJsonText) const
{
    try {
        const json& j = json::parse(geoJsonText);
        const std::string& type = j.at("type");
        if (type == "Feature") {
            return readFeatureForGeometry(j);
        }
        else if (type == "FeatureCollection") {
            return readFeatureCollectionForGeometry(j);
        }
        else {
            return readGeometry(j);
        }
    }
    catch (json::exception& ex) {
        throw ParseException("Error parsing JSON", ex.what());
    }
}

GeoJSONFeatureCollection GeoJSONReader::readFeatures(const std::string& geoJsonText) const
{
    try {
        const json& j = json::parse(geoJsonText);
        const std::string& type = j.at("type");
        if (type == "Feature") {
            const auto& feature = readFeature(j);
            return GeoJSONFeatureCollection { std::vector<GeoJSONFeature>{feature} };
        }
        else if (type == "FeatureCollection") {
            return readFeatureCollection(j);
        }
        else {
            auto g = readGeometry(j);
            return GeoJSONFeatureCollection { std::vector<GeoJSONFeature>{GeoJSONFeature{std::move(g), std::map<std::string, GeoJSONValue>{} }}};
        }
    }
    catch (json::exception& ex) {
        throw ParseException("Error parsing JSON", ex.what());
    }
}

std::unique_ptr<geom::Geometry> GeoJSONReader::readFeatureForGeometry(
    const geos_nlohmann::json& j) const
{
    const auto& geometryJson = j.at("geometry");
    auto geometry = readGeometry(geometryJson);
    return geometry;
}

GeoJSONFeature GeoJSONReader::readFeature(const geos_nlohmann::json& j) const
{
    const auto& geometryJson = j.at("geometry");
    const auto& properties = j.at("properties");
    return GeoJSONFeature{readGeometry(geometryJson), readProperties(properties)};
}

std::map<std::string, GeoJSONValue> GeoJSONReader::readProperties(
    const geos_nlohmann::json& p) const
{
    std::map<std::string, GeoJSONValue> map;
    for (const auto& prop : p.items()) {
        map[prop.key()] = std::move(readProperty(prop.value()));
    }
    return map;
}

GeoJSONValue GeoJSONReader::readProperty(
    const geos_nlohmann::json& value) const
{
    if (value.is_string()) {
        return GeoJSONValue { value.get<std::string>() };
    }
    else if (value.is_number()) {
        return GeoJSONValue { value.get<double>() };
    }
    else if (value.is_boolean()) {
        return GeoJSONValue { value.get<bool>() };
    }
    else if (value.is_array()) {
        std::vector<GeoJSONValue> v {};
        v.reserve(value.size());
        for (const auto& el : value.items()) {
            v.push_back(readProperty(el.value()));
        }
        return GeoJSONValue{ v };
    }
    else if (value.is_object()) {
        std::map<std::string, GeoJSONValue> v {};
        for (const auto& el : value.items()) {
            v[el.key()] = std::move(readProperty(el.value()));
        }
        return GeoJSONValue{ v };
    }
    else {
        return GeoJSONValue{};
    }
}

std::unique_ptr<geom::Geometry> GeoJSONReader::readFeatureCollectionForGeometry(
    const geos_nlohmann::json& j) const
{
    const auto& featuresJson = j.at("features");
    std::vector<std::unique_ptr<geom::Geometry>> geometries;
    geometries.reserve(featuresJson.size());
    for (const auto& featureJson : featuresJson) {
        auto g = readFeatureForGeometry(featureJson);
        geometries.push_back(std::move(g));
    }
    return geometryFactory.createGeometryCollection(std::move(geometries));
}

GeoJSONFeatureCollection GeoJSONReader::readFeatureCollection(
    const geos_nlohmann::json& j) const
{
    const auto& featuresJson = j.at("features");
    std::vector<GeoJSONFeature> features;
    features.reserve(featuresJson.size());
    for (const auto& featureJson : featuresJson) {
        features.push_back(readFeature(featureJson));
    }
    return GeoJSONFeatureCollection{std::move(features)};
}


std::unique_ptr<geom::Geometry> GeoJSONReader::readGeometry(
    const geos_nlohmann::json& j) const
{
    const std::string& type = j.at("type");
    if (type == "Point") {
        return readPoint(j);
    }
    else if (type == "LineString") {
        return readLineString(j);
    }
    else if (type == "Polygon") {
        return readPolygon(j);
    }
    else if (type == "MultiPoint") {
        return readMultiPoint(j);
    }
    else if (type == "MultiLineString") {
        return readMultiLineString(j);
    }
    else if (type == "MultiPolygon") {
        return readMultiPolygon(j);
    }
    else if (type == "GeometryCollection") {
        return readGeometryCollection(j);
    }
    else {
        throw ParseException{"Unknown geometry type!"};
    }
}

geom::Coordinate GeoJSONReader::readCoordinate(
    const std::vector<double>& coords) const
{
    if (coords.size() == 1) {
        throw  ParseException("Expected two coordinates found one");
    }
    else if (coords.size() > 2) {
        throw  ParseException("Expected two coordinates found more than two");
    }
    else {
        return geom::Coordinate {coords[0], coords[1]};
    }
}

std::unique_ptr<geom::Point> GeoJSONReader::readPoint(
    const geos_nlohmann::json& j) const
{
    const auto& coords = j.at("coordinates").get<std::vector<double>>();
    if (coords.size() == 1) {
        throw  ParseException("Expected two coordinates found one");
    }
    else if (coords.size() < 2) {
        return geometryFactory.createPoint(2);
    }
    else {
        const geom::Coordinate& coord = readCoordinate(coords);
        return std::unique_ptr<geom::Point>(geometryFactory.createPoint(coord));
    }
}

std::unique_ptr<geom::LineString> GeoJSONReader::readLineString(
    const geos_nlohmann::json& j) const
{
    const auto& coords = j.at("coordinates").get<std::vector<std::vector<double>>>();
    auto coordinates = detail::make_unique<CoordinateSequence>(0u, 2u);
    coordinates->reserve(coords.size());
    for (const auto& coord : coords) {
        const geom::Coordinate& c = readCoordinate(coord);
        coordinates->add(c);
    }
    return geometryFactory.createLineString(std::move(coordinates));
}

std::unique_ptr<geom::Polygon> GeoJSONReader::readPolygon(
    const geos_nlohmann::json& j) const
{
    const auto& polygonCoords = j.at("coordinates").get<std::vector<std::vector<std::vector<double>>>>();
    return readPolygon(polygonCoords);
}

std::unique_ptr<geom::Polygon> GeoJSONReader::readPolygon(
    const std::vector<std::vector<std::vector<double>>>& polygonCoords) const
{
    std::unique_ptr<geom::LinearRing> shell;
    std::vector<std::unique_ptr<geom::LinearRing>> rings;
    rings.reserve(polygonCoords.size());
    for (const auto& ring : polygonCoords) {
    auto coordinates = detail::make_unique<CoordinateSequence>(0u, 2u);
        coordinates->reserve(ring.size());
        for (const auto& coord : ring) {
            const geom::Coordinate& c = readCoordinate(coord);
            coordinates->add(c);
        }
        if (!shell) {
            shell = geometryFactory.createLinearRing(std::move(coordinates));
        }
        else {
            rings.push_back(geometryFactory.createLinearRing(std::move(coordinates)));
        }
    }
    if (!shell) {
        return geometryFactory.createPolygon(2);
    }
    else if (rings.size() == 0) {
        return geometryFactory.createPolygon(std::move(shell));
    }
    else {
        return geometryFactory.createPolygon(std::move(shell), std::move(rings));
    }
}

std::unique_ptr<geom::MultiPoint> GeoJSONReader::readMultiPoint(
    const geos_nlohmann::json& j) const
{
    const auto& coords = j.at("coordinates").get<std::vector<std::vector<double>>>();
    std::vector<std::unique_ptr<geom::Point>> points;
    points.reserve(coords.size());
    for (const auto& coord : coords) {
        const geom::Coordinate& c = readCoordinate(coord);
        points.push_back(std::unique_ptr<geom::Point>(geometryFactory.createPoint(c)));
    }
    return geometryFactory.createMultiPoint(std::move(points));
}

std::unique_ptr<geom::MultiLineString> GeoJSONReader::readMultiLineString(
    const geos_nlohmann::json& j) const
{
    const auto& listOfCoords = j.at("coordinates").get<std::vector<std::vector<std::vector<double>>>>();
    std::vector<std::unique_ptr<geom::LineString>> lines;
    lines.reserve(listOfCoords.size());
    for (const auto& coords :  listOfCoords) {
        auto coordinates = detail::make_unique<geom::CoordinateSequence>(0u, 2u);
        coordinates->reserve(coords.size());
        for (const auto& coord : coords) {
            const geom::Coordinate& c = readCoordinate(coord);
            coordinates->add(geom::Coordinate{c.x, c.y});
        }
        lines.push_back(geometryFactory.createLineString(std::move(coordinates)));
    }
    return geometryFactory.createMultiLineString(std::move(lines));
}

std::unique_ptr<geom::MultiPolygon> GeoJSONReader::readMultiPolygon(
    const geos_nlohmann::json& j) const
{
    const auto& multiPolygonCoords = j.at("coordinates").get<std::vector<std::vector<std::vector<std::vector<double>>>>>();
    std::vector<std::unique_ptr<geom::Polygon>> polygons;
    polygons.reserve(multiPolygonCoords.size());
    for (const auto& polygonCoords : multiPolygonCoords) {
        polygons.push_back(readPolygon(polygonCoords));
    }
    return geometryFactory.createMultiPolygon(std::move(polygons));
}

std::unique_ptr<geom::GeometryCollection> GeoJSONReader::readGeometryCollection(
    const geos_nlohmann::json& j) const
{
    const auto& jsonGeometries = j.at("geometries");
    std::vector<std::unique_ptr<geom::Geometry>> geometries;
    geometries.reserve(jsonGeometries.size());
    for (const auto& jsonGeometry : jsonGeometries) {
        auto g = readGeometry(jsonGeometry);
        geometries.push_back(std::move(g));
    }
    return geometryFactory.createGeometryCollection(std::move(geometries));
}

} // namespace geos.io
} // namespace geos
