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

#ifndef GEOS_IO_GEOJSONWRITER_H
#define GEOS_IO_GEOJSONWRITER_H

#include <geos/export.h>

#include "GeoJSON.h"
#include <string>
#include <cctype>
#include "geos/vend/include_nlohmann_json.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Geometry;
class GeometryCollection;
class Point;
class LineString;
class LinearRing;
class Polygon;
class MultiPoint;
class MultiLineString;
class MultiPolygon;
class PrecisionModel;
}
namespace io {
class Writer;
}
}


namespace geos {
namespace io {

enum class GeoJSONType {
    GEOMETRY, FEATURE, FEATURE_COLLECTION
};

/**
 * \class GeoJSONWriter
 *
 * \brief Outputs the GeoJSON representation of a Geometry.
 * See also GeoJSONReader for parsing.
 *
 */
class GEOS_DLL GeoJSONWriter {
public:
    ~GeoJSONWriter() = default;

    std::string write(const geom::Geometry* geometry, GeoJSONType type = GeoJSONType::GEOMETRY);

    std::string writeFormatted(const geom::Geometry* geometry, GeoJSONType type = GeoJSONType::GEOMETRY, int indent = 4);

    std::string write(const GeoJSONFeature& feature);

    std::string write(const GeoJSONFeatureCollection& features);

private:

    std::pair<double, double> convertCoordinate(const geom::Coordinate* c);

    std::vector<std::pair<double, double>> convertCoordinateSequence(const geom::CoordinateSequence* c);

    void encode(const geom::Geometry* g, GeoJSONType type, geos_nlohmann::ordered_json& j);

    void encodeGeometry(const geom::Geometry* g, geos_nlohmann::ordered_json& j);

    void encodePoint(const geom::Point* p, geos_nlohmann::ordered_json& j);

    void encodeLineString(const geom::LineString* l, geos_nlohmann::ordered_json& j);

    void encodePolygon(const geom::Polygon* p, geos_nlohmann::ordered_json& j);

    void encodeMultiPoint(const geom::MultiPoint* p, geos_nlohmann::ordered_json& j);

    void encodeMultiLineString(const geom::MultiLineString* l, geos_nlohmann::ordered_json& j);

    void encodeMultiPolygon(const geom::MultiPolygon* m, geos_nlohmann::ordered_json& j);

    void encodeGeometryCollection(const geom::GeometryCollection* g, geos_nlohmann::ordered_json& j);

    void encodeFeature(const geom::Geometry* g, geos_nlohmann::ordered_json& j);

    void encodeFeatureCollection(const geom::Geometry* g, geos_nlohmann::ordered_json& j);

    void encodeFeature(const GeoJSONFeature& feature, geos_nlohmann::ordered_json& j);

    void encodeGeoJSONValue(const std::string& key, const GeoJSONValue& value, geos_nlohmann::ordered_json& j);

};

} // namespace geos::io
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // #ifndef GEOS_IO_GEOJSONWRITER_H
