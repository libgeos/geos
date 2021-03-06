/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_IO_GEOJSON_H
#define GEOS_IO_GEOJSON_H

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
        std::string getString() const;
        std::nullptr_t getNull() const;
        bool getBoolean() const;
        std::map<std::string,GeoJSONValue> getObject() const;
        std::vector<GeoJSONValue> getArray() const;

        bool isNumber() const;
        bool isString() const;
        bool isNull() const;
        bool isBoolean() const;
        bool isObject() const;
        bool isArray() const;

};

class GEOS_DLL GeoJSONFeature {

    public:

        GeoJSONFeature(std::unique_ptr<geom::Geometry> g, std::map<std::string, GeoJSONValue> p);

        GeoJSONFeature(GeoJSONFeature const &other);

        geom::Geometry* getGeometry() const;

        std::map<std::string, GeoJSONValue> getProperties() const;

    private:

        std::unique_ptr<geom::Geometry> geometry;

        std::map<std::string, GeoJSONValue> properties;

};

class GEOS_DLL GeoJSONFeatureCollection {

    public:

        GeoJSONFeatureCollection(std::vector<GeoJSONFeature> f);

        std::vector<GeoJSONFeature> getFeatures() const; 

    private:

        std::vector<GeoJSONFeature> features;

};

} // namespace geos::io
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // #ifndef GEOS_IO_GEOJSON_H
