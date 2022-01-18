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

#pragma once

#include <geos/export.h>

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <cstddef>
#include <geos/geom/Geometry.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace geom {
class Geometry;
}
}

namespace geos {
namespace io {

class GEOS_DLL GeoJSONValue {

private:

    enum class Type { NUMBER, STRING, NULLTYPE, BOOLEAN, OBJECT, ARRAY };

    Type type;

    union {
        double d;
        std::string s;
        std::nullptr_t n;
        bool b;
        std::map<std::string, GeoJSONValue> o;
        std::vector<GeoJSONValue> a;
    };

    void cleanup();

public:

    struct GeoJSONTypeError {};

    GeoJSONValue(double);
    GeoJSONValue(const std::string&);
    GeoJSONValue();
    GeoJSONValue(bool);
    GeoJSONValue(const std::map<std::string, GeoJSONValue>&);
    GeoJSONValue(const std::vector<GeoJSONValue>&);

    ~GeoJSONValue();
    GeoJSONValue(const GeoJSONValue&);
    GeoJSONValue& operator=(const GeoJSONValue&);

    double getNumber() const;
    const std::string& getString() const;
    std::nullptr_t getNull() const;
    bool getBoolean() const;
    const std::map<std::string, GeoJSONValue>& getObject() const;
    const std::vector<GeoJSONValue>& getArray() const;

    bool isNumber() const;
    bool isString() const;
    bool isNull() const;
    bool isBoolean() const;
    bool isObject() const;
    bool isArray() const;

};

class GEOS_DLL GeoJSONFeature {

public:

    GeoJSONFeature(std::unique_ptr<geom::Geometry> g,
                   const std::map<std::string, GeoJSONValue>& p);

    GeoJSONFeature(std::unique_ptr<geom::Geometry> g,
                   std::map<std::string, GeoJSONValue>&& p);

    GeoJSONFeature(GeoJSONFeature const& other);

    GeoJSONFeature(GeoJSONFeature&& other);

    GeoJSONFeature& operator=(const GeoJSONFeature&);

    GeoJSONFeature& operator=(GeoJSONFeature&&);

    const geom::Geometry* getGeometry() const;

    const std::map<std::string, GeoJSONValue>& getProperties() const;

private:

    std::unique_ptr<geom::Geometry> geometry;

    std::map<std::string, GeoJSONValue> properties;

};

class GEOS_DLL GeoJSONFeatureCollection {

public:

    GeoJSONFeatureCollection(const std::vector<GeoJSONFeature>& f);

    GeoJSONFeatureCollection(std::vector<GeoJSONFeature>&& f);

    const std::vector<GeoJSONFeature>& getFeatures() const;

private:

    std::vector<GeoJSONFeature> features;

};

} // namespace geos::io
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

