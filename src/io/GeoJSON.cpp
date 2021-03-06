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

GeoJSONValue::GeoJSONValue(double value) {
    type = Type::NUMBER;
    d = value;
}

GeoJSONValue::GeoJSONValue(const std::string& value)  {
    type = Type::STRING;
    new(&s) std::string{value};
}

GeoJSONValue::GeoJSONValue()  {
    type = Type::NULLTYPE;
    n = nullptr;
}

GeoJSONValue::GeoJSONValue(bool value)  {
    type = Type::BOOLEAN;
    b = value;
}

GeoJSONValue::GeoJSONValue(const std::map<std::string, GeoJSONValue>& value)  {
    type = Type::OBJECT;
    new(&o) std::map<std::string, GeoJSONValue>{value};
}

GeoJSONValue::GeoJSONValue(const std::vector<GeoJSONValue>& value)  {
    type = Type::ARRAY;
    new(&a) std::vector<GeoJSONValue>{value};
}

void GeoJSONValue::cleanup() {
    using std::string;
    using object = std::map<std::string, GeoJSONValue>;
    using array = std::vector<GeoJSONValue>;
    if (type == Type::STRING) {
         s.~string();
    } else if (type == Type::OBJECT) {
        o.~object();
    } else if (type == Type::ARRAY) { 
        a.~array();
    }
}

GeoJSONValue::~GeoJSONValue() {
    cleanup();
}

GeoJSONValue::GeoJSONValue(const GeoJSONValue& v) {
    switch(v.type) {
        case Type::NUMBER:
            d = v.d;
            break;
        case Type::BOOLEAN:
            b = v.b;
            break;
        case Type::STRING:
            new(&s) std::string{v.s};
            break;
        case Type::NULLTYPE:
            n = v.n;
            break;
        case Type::OBJECT:
            new(&o) std::map<std::string, GeoJSONValue>{v.o};
            break;
        case Type::ARRAY:
            new(&a) std::vector<GeoJSONValue>{v.a};
            break;                
    }
    type = v.type;
}

GeoJSONValue& GeoJSONValue::operator=(const GeoJSONValue& v) {
    if (type == Type::STRING && v.type == Type::STRING) {
        s = v.s;
        return *this;
    } else if (type == Type::OBJECT && v.type == Type::OBJECT) {
        o = v.o;
        return *this;
    } else if (type == Type::ARRAY && v.type == Type::ARRAY) {
        a = v.a;
        return *this;
    } 

    cleanup();

    switch(v.type) {
        case Type::NUMBER:
            d = v.d;
            break;
        case Type::BOOLEAN:
            b = v.b;
            break;
        case Type::STRING:
            new(&s) std::string{v.s};
            break;
        case Type::NULLTYPE:
            n = v.n;
            break;
        case Type::OBJECT:
            new(&o) std::map<std::string, GeoJSONValue>{v.o};
            break;
        case Type::ARRAY:
            new(&a) std::vector<GeoJSONValue>{v.a};
            break;                
    }
    type = v.type;
    return *this;
}

double GeoJSONValue::getNumber() const {
    if (type != Type::NUMBER) throw GeoJSONTypeError{};
    return d;

}
std::string GeoJSONValue::getString() const {
    if (type != Type::STRING) throw GeoJSONTypeError{};
    return s;
}

std::nullptr_t GeoJSONValue::getNull() const {
    if (type != Type::NULLTYPE) throw GeoJSONTypeError{};
    return n;
}

bool GeoJSONValue::getBoolean() const {
    if (type != Type::BOOLEAN) throw GeoJSONTypeError{};
    return b;
}

std::map<std::string,GeoJSONValue> GeoJSONValue::getObject() const {
    if (type != Type::OBJECT) throw GeoJSONTypeError{};
    return o;
}

std::vector<GeoJSONValue> GeoJSONValue::getArray() const {
    if (type != Type::ARRAY) throw GeoJSONTypeError{};
    return a;
}

bool GeoJSONValue::isNumber() const {
    return type == Type::NUMBER;
}

bool GeoJSONValue::isString() const {
    return type == Type::STRING;
}

bool GeoJSONValue::isNull() const {
    return type == Type::NULLTYPE;
}

bool GeoJSONValue::isBoolean() const {
    return type == Type::BOOLEAN;
}

bool GeoJSONValue::isObject() const {
    return type == Type::OBJECT;
}

bool GeoJSONValue::isArray() const {
    return type == Type::ARRAY;
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
