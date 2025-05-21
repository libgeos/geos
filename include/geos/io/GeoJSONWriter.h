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

    /*
     * \brief
     * Returns the output dimension used by the
     * <code>GeoJSONWriter</code>.
     */
    int
    getOutputDimension() const
    {
        return defaultOutputDimension;
    }

    /*
     * Sets the output dimension used by the <code>GeoJSONWriter</code>.
     *
     * @param newOutputDimension Supported values are 2 or 3.
     *        Default since GEOS 3.12 is 3.
     *        Note that 3 indicates up to 3 dimensions will be
     *        written but 2D GeoJSON is still produced for 2D geometries.
     */
    void setOutputDimension(uint8_t newOutputDimension);

    /*
     * Sets whether the GeoJSON should be output following counter-clockwise orientation aka Right Hand Rule defined in
     * RFC7946. See <a href="https://tools.ietf.org/html/rfc7946#section-3.1.6">RFC 7946 Specification</a>
     * for more context.
     *
     * @param newIsForceCCW true if the GeoJSON should be output following the RFC7946
     *                      counter-clockwise orientation aka Right Hand Rule
     */
    void setForceCCW(bool newIsForceCCW);

private:
    uint8_t defaultOutputDimension = 3;

    bool isForceCCW = false;

    std::vector<double> convertCoordinate(const geom::Coordinate* c);

    std::vector<std::vector<double>> convertCoordinateSequence(const geom::CoordinateSequence* c);

    std::vector<std::vector<std::vector<double>>> convertLinearRings(const geom::Polygon *poly);

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

