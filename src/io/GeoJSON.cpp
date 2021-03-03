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

#include <geos/io/GeoJSON.h>

#undef DEBUG_GEOJSON

namespace geos {
namespace io { // geos.io

// GeoJSONValue

GeoJSONValue GeoJSONValue::createStringValue(std::string s) {
    GeoJSONValue value{};
    value.type = GeoJSONValueType::STRING;
    value.stringValue = s;
    return value;
}

GeoJSONValue GeoJSONValue::createNumberValue(double n) {
    GeoJSONValue value{};
    value.type = GeoJSONValueType::NUMBER;
    value.numberValue = n;
    return value;
}

GeoJSONValue GeoJSONValue::createNullValue() {
    GeoJSONValue value{};
    value.type = GeoJSONValueType::NULLTYPE;
    value.nullValue = nullptr;
    return value;
}

GeoJSONValue GeoJSONValue::createBooleanValue(bool b) {
    GeoJSONValue value{};
    value.type = GeoJSONValueType::BOOLEAN;
    value.booleanValue = b;
    return value;
}   

GeoJSONValue GeoJSONValue::createObjectValue() {
    GeoJSONValue value{};
    value.type = GeoJSONValueType::OBJECT;
    return value;
}

GeoJSONValue GeoJSONValue::createArrayValue() {
    GeoJSONValue value{};
    value.type = GeoJSONValueType::ARRAY;
    return value;
}

std::string GeoJSONValue::toString() const {
    std::stringstream os;
    if (type == GeoJSONValueType::STRING) {
        os << stringValue;
    } else if (type == GeoJSONValueType::NUMBER) {
        os << numberValue;
    } else if (type == GeoJSONValueType::BOOLEAN) {
        os << booleanValue;
    } else if (type == GeoJSONValueType::NULLTYPE) {
        os << "null";
    } else if (type == GeoJSONValueType::ARRAY) {
        bool first = true;
        for(auto item : arrayValue) {
            if (!first) {
                os << ", ";
            }
            os << item.toString();
            first = false;
        }
    } else if (type == GeoJSONValueType::OBJECT) {
        bool first = true;
        for(auto item : objectValue) {
            if (!first) {
                os << ", ";
            }
            os << item.first << " = " << item.second.toString();
            first = false;
        }
    }
    return os.str();
}

std::ostream& operator << (std::ostream& os, const GeoJSONValue& val) {
    os << val.toString();
    return os;
}

// GeoJSONFeature

GeoJSONFeature::GeoJSONFeature(std::unique_ptr<geom::Geometry> g, std::map<std::string, GeoJSONValue> p) : geometry(std::move(g)), properties(p) {}

GeoJSONFeature::GeoJSONFeature(GeoJSONFeature const &other) : geometry(other.geometry->clone()), properties(other.properties) {}

geom::Geometry* GeoJSONFeature::getGeometry() const {
    return geometry.get();
}

std::map<std::string, GeoJSONValue> GeoJSONFeature::getProperties() const {
    return properties;
}

// GeoJSONFeatureCollection

GeoJSONFeatureCollection::GeoJSONFeatureCollection(std::vector<GeoJSONFeature> f) : features(f) {};

std::vector<GeoJSONFeature> GeoJSONFeatureCollection::getFeatures() const {
    return features;
}

} // namespace geos.io
} // namespace geos
