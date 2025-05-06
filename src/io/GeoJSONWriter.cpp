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

#include <geos/io/GeoJSONWriter.h>
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
#include <geos/geom/PrecisionModel.h>

#include <algorithm>
#include <ostream>
#include <sstream>
#include <cassert>
#include <cmath>

#include "geos/util.h"

#define GEOS_COMPILATION

using namespace geos::geom;
using json = geos_nlohmann::ordered_json;

namespace geos {
namespace io { // geos.io


/* public */
void
GeoJSONWriter::setOutputDimension(uint8_t dims)
{
    if(dims < 2 || dims > 3) {
        throw util::IllegalArgumentException("GeoJSON output dimension must be 2 or 3");
    }
    defaultOutputDimension = dims;
}

void GeoJSONWriter::setForceCCW(bool newIsForceCCW) {
    isForceCCW = newIsForceCCW;
}

std::string GeoJSONWriter::write(const geom::Geometry* geometry, GeoJSONType type)
{
    json j;
    encode(geometry, type, j);
    return j.dump();
}

std::string GeoJSONWriter::writeFormatted(const geom::Geometry* geometry, GeoJSONType type, int indent)
{
    json j;
    encode(geometry, type, j);
    return j.dump(indent);
}

std::string GeoJSONWriter::write(const GeoJSONFeature& feature)
{
    json j;
    encodeFeature(feature, j);
    return j.dump();
}

void GeoJSONWriter::encodeGeoJSONValue(const std::string& key, const GeoJSONValue& value,
                                       geos_nlohmann::ordered_json& j)
{
    if (value.isNumber()) {
        if (j.is_object()) {
            j[key] = value.getNumber();
        }
        else {
            j.push_back(value.getNumber());
        }
    }
    else if (value.isString()) {
        if (j.is_object()) {
            j[key] = value.getString();
        }
        else {
            j.push_back(value.getString());
        }
    }
    else if (value.isBoolean()) {
        if (j.is_object()) {
            j[key] = value.getBoolean();
        }
        else {
            j.push_back(value.getBoolean());
        }
    }
    else if (value.isNull()) {
        if (j.is_object()) {
            j[key] = nullptr;
        }
        else {
            j.push_back(nullptr);
        }
    }
    else if (value.isArray()) {
        if (j.is_object()) {
          j[key] = json::array();
          for (const GeoJSONValue& v : value.getArray()) {
              encodeGeoJSONValue("", v, j[key]);
          }
        }
        else {
          json sub_array = json::array();
          for (const GeoJSONValue& v : value.getArray()) {
              encodeGeoJSONValue("", v, sub_array);
          }
          j.push_back(sub_array);
        }
    }
    else if (value.isObject()) {
        if (j.is_object()) {
          j[key] = json::object();
          for (const auto& entry : value.getObject()) {
              encodeGeoJSONValue(entry.first, entry.second, j[key]);
          }
        }
        else {
          json sub_obj = json::object();
          for (const auto& entry : value.getObject()) {
              encodeGeoJSONValue(entry.first, entry.second, sub_obj);
          }
          j.push_back(sub_obj);
        }
    }
}

std::string GeoJSONWriter::write(const GeoJSONFeatureCollection& features)
{
    json j;
    j["type"] = "FeatureCollection";
    json featuresJson = json::array();
    for (auto const& feature : features.getFeatures()) {
        json featureJson;
        encodeFeature(feature, featureJson);
        featuresJson.push_back(featureJson);
    }
    j["features"] = featuresJson;
    return j.dump();
}

void GeoJSONWriter::encodeFeature(const GeoJSONFeature& feature, geos_nlohmann::ordered_json& j)
{
    j["type"] = "Feature";

    if (feature.getId().size() > 0) j["id"] = feature.getId();

    json geometryJson;
    encodeGeometry(feature.getGeometry(), geometryJson);
    j["geometry"] = geometryJson;

    json propertiesJson = json::object();
    for (auto const& property : feature.getProperties()) {
        std::string key = property.first;
        GeoJSONValue value = property.second;
        encodeGeoJSONValue(key, value, propertiesJson);
    }
    j["properties"] = propertiesJson;
}

void GeoJSONWriter::encode(const geom::Geometry* geometry, GeoJSONType geojsonType, geos_nlohmann::ordered_json& j)
{
    if (geojsonType == GeoJSONType::GEOMETRY) {
        encodeGeometry(geometry, j);
    }
    else if (geojsonType == GeoJSONType::FEATURE) {
        encodeFeature(geometry, j);
    }
    else if (geojsonType == GeoJSONType::FEATURE_COLLECTION) {
        encodeFeatureCollection(geometry, j);
    }
}

void GeoJSONWriter::encodeFeature(const geom::Geometry* g, geos_nlohmann::ordered_json& j)
{
    json geometryJson;
    encodeGeometry(g, geometryJson);
    j["type"] = "Feature";
    j["geometry"] = geometryJson;
}

void GeoJSONWriter::encodeFeatureCollection(const geom::Geometry* g, geos_nlohmann::ordered_json& j)
{
    json featureJson;
    encodeFeature(g, featureJson);
    std::vector<json> features;
    features.push_back(featureJson);
    j["type"] = "FeatureCollection";
    j["features"] = features;
}

void GeoJSONWriter::encodeGeometry(const geom::Geometry* geometry, geos_nlohmann::ordered_json& j)
{
    util::ensureNoCurvedComponents(geometry);

    auto type = geometry->getGeometryTypeId();
    if (type == GEOS_POINT) {
        auto point = static_cast<const geom::Point*>(geometry);
        encodePoint(point, j);
    }
    else if (type == GEOS_LINESTRING) {
        auto line = static_cast<const geom::LineString*>(geometry);
        encodeLineString(line, j);
    }
    else if (type == GEOS_LINEARRING) {
        auto line = static_cast<const geom::LineString*>(geometry);
        encodeLineString(line, j);
    }
    else if (type == GEOS_POLYGON) {
        auto poly = static_cast<const geom::Polygon*>(geometry);
        if (isForceCCW) {
            auto ccwPoly = poly->clone();
            ccwPoly->orientRings(false);
            encodePolygon(ccwPoly.get(), j);
        } else {
            encodePolygon(poly, j);
        }
    }
    else if (type == GEOS_MULTIPOINT) {
        auto multiPoint = static_cast<const geom::MultiPoint*>(geometry);
        encodeMultiPoint(multiPoint, j);
    }
    else if (type == GEOS_MULTILINESTRING) {
        auto multiLineString = static_cast<const geom::MultiLineString*>(geometry);
        encodeMultiLineString(multiLineString, j);
    }
    else if (type == GEOS_MULTIPOLYGON) {
        auto multiPolygon = static_cast<const geom::MultiPolygon*>(geometry);
        encodeMultiPolygon(multiPolygon, j);
    }
    else if (type == GEOS_GEOMETRYCOLLECTION) {
        auto geometryCollection = static_cast<const geom::GeometryCollection*>(geometry);
        encodeGeometryCollection(geometryCollection, j);
    }
}

void GeoJSONWriter::encodePoint(const geom::Point* point, geos_nlohmann::ordered_json& j)
{
    j["type"] = "Point";
    if (!point->isEmpty()) {
        auto as_coord = Coordinate { point->getX(), point->getY(), point->getZ()};
        j["coordinates"] = convertCoordinate(&as_coord);
    }
    else {
        j["coordinates"] = j.array();
    }
}

void GeoJSONWriter::encodeLineString(const geom::LineString* line, geos_nlohmann::ordered_json& j)
{
    j["type"] = "LineString";
    j["coordinates"] = convertCoordinateSequence(line->getCoordinates().get());
}

void GeoJSONWriter::encodePolygon(const geom::Polygon* poly, geos_nlohmann::ordered_json& j)
{
    j["type"] = "Polygon";
    j["coordinates"] = convertLinearRings(poly);
}

void GeoJSONWriter::encodeMultiPoint(const geom::MultiPoint* multiPoint, geos_nlohmann::ordered_json& j)
{
    j["type"] = "MultiPoint";
    j["coordinates"] = convertCoordinateSequence(multiPoint->getCoordinates().get());
}

void GeoJSONWriter::encodeMultiLineString(const geom::MultiLineString* multiLineString, geos_nlohmann::ordered_json& j)
{
    j["type"] = "MultiLineString";
    std::vector<std::vector<std::vector<double>>> lines;
    lines.reserve(multiLineString->getNumGeometries());
    for (size_t i = 0; i < multiLineString->getNumGeometries(); i++) {
        lines.push_back(convertCoordinateSequence(multiLineString->getGeometryN(i)->getCoordinates().get()));
    }
    j["coordinates"] = lines;
}

void GeoJSONWriter::encodeMultiPolygon(const geom::MultiPolygon* multiPolygon, geos_nlohmann::ordered_json& json)
{
    json["type"] = "MultiPolygon";
    std::vector<std::vector<std::vector<std::vector<double>>>> polygons;
    polygons.reserve(multiPolygon->getNumGeometries());
    for (size_t i = 0; i < multiPolygon->getNumGeometries(); i++) {
        const Polygon* polygon = multiPolygon->getGeometryN(i);
        if (isForceCCW) {
            auto ccwPolygon = polygon->clone();
            ccwPolygon->orientRings(false);
            polygons.push_back(convertLinearRings(ccwPolygon.get()));
        } else {
            polygons.push_back(convertLinearRings(polygon));
        }
    }
    json["coordinates"] = polygons;
}

void GeoJSONWriter::encodeGeometryCollection(const geom::GeometryCollection* g, geos_nlohmann::ordered_json& j)
{
    j["type"] = "GeometryCollection";
    auto geometryArray = j.array();
    for (size_t i = 0; i < g->getNumGeometries(); i++) {
        auto geometryObj = j.object();
        encodeGeometry(g->getGeometryN(i), geometryObj);
        geometryArray.push_back(geometryObj);
    }
    j["geometries"] = geometryArray;
}

std::vector<double> GeoJSONWriter::convertCoordinate(const Coordinate* c)
{
    if (std::isnan(c->z) || defaultOutputDimension == 2) {
        return std::vector<double> { c->x, c->y };
    }
    return std::vector<double> { c->x, c->y, c->z };
}

std::vector<std::vector<double>> GeoJSONWriter::convertCoordinateSequence(const CoordinateSequence*
                                    coordinateSequence)
{
    std::vector<std::vector<double>> coordinates;
    coordinates.reserve(coordinateSequence->size());
    for (size_t i = 0; i<coordinateSequence->size(); i++) {
        const geom::Coordinate& c = coordinateSequence->getAt(i);
        coordinates.push_back(convertCoordinate(&c));
    }
    return coordinates;
}

std::vector<std::vector<std::vector<double>>> GeoJSONWriter::convertLinearRings(const geom::Polygon* poly)
{
    std::vector<std::vector<std::vector<double>>> rings;
    auto ring = poly->getExteriorRing();
    rings.reserve(poly->getNumInteriorRing() + 1);
    rings.push_back(convertCoordinateSequence(ring->getCoordinates().get()));
    for (size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        rings.push_back(convertCoordinateSequence(poly->getInteriorRingN(i)->getCoordinates().get()));
    }
    return rings;
}

} // namespace geos.io
} // namespace geos
