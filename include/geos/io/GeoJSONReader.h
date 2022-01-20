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

#include <geos/io/GeoJSON.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <string>
#include "geos/vend/include_nlohmann_json.hpp"

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
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
}

namespace geos {
namespace io {

/**
 * \class GeoJSONReader
 * \brief GeoJSON reader class; see also GeoJSONWriter.
 */
class GEOS_DLL GeoJSONReader {
public:

    /**
     * \brief Inizialize parser with given GeometryFactory.
     *
     * Note that all Geometry objects created by the
     * parser will contain a pointer to the given factory
     * so be sure you'll keep the factory alive for the
     * whole GeoJSONReader and created Geometry life.
     */
    GeoJSONReader(const geom::GeometryFactory& gf);

    /**
     * \brief Inizialize parser with default GeometryFactory.
     *
     */
    GeoJSONReader();

    ~GeoJSONReader() = default;

    /// Parse a GeoJSON string returning a Geometry
    std::unique_ptr<geom::Geometry> read(const std::string& geoJsonText) const;

    GeoJSONFeatureCollection readFeatures(const std::string& geoJsonText) const;

private:

    const geom::GeometryFactory& geometryFactory;

    std::unique_ptr<geom::Geometry> readFeatureForGeometry(const geos_nlohmann::json& j) const;

    GeoJSONFeature readFeature(const geos_nlohmann::json& j) const;

    std::map<std::string, GeoJSONValue> readProperties(const geos_nlohmann::json& p) const;

    GeoJSONValue readProperty(const geos_nlohmann::json& p) const;

    std::unique_ptr<geom::Geometry> readFeatureCollectionForGeometry(
        const geos_nlohmann::json& j) const;

    GeoJSONFeatureCollection readFeatureCollection(
        const geos_nlohmann::json& j) const;

    std::unique_ptr<geom::Geometry> readGeometry(
        const geos_nlohmann::json& j) const;

    std::unique_ptr<geom::Point> readPoint(const geos_nlohmann::json& j) const;

    geom::Coordinate readCoordinate(const std::vector<double>& coords) const;

    std::unique_ptr<geom::LineString> readLineString(
        const geos_nlohmann::json& j) const;

    std::unique_ptr<geom::Polygon> readPolygon(
        const geos_nlohmann::json& j) const;

    std::unique_ptr<geom::Polygon> readPolygon(
        const std::vector<std::vector<std::vector<double>>>& c) const;

    std::unique_ptr<geom::MultiPoint> readMultiPoint(
        const geos_nlohmann::json& j) const;

    std::unique_ptr<geom::MultiLineString> readMultiLineString(
        const geos_nlohmann::json& j) const;

    std::unique_ptr<geom::MultiPolygon> readMultiPolygon(
        const geos_nlohmann::json& j) const;

    std::unique_ptr<geom::GeometryCollection> readGeometryCollection(
        const geos_nlohmann::json& j) const;

};

} // namespace io
} // namespace geos

