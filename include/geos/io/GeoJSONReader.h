/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_IO_GEOJSONREADER_H
#define GEOS_IO_GEOJSONREADER_H

#include <geos/export.h>

#include <geos/io/GeoJSON.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <string>
#include <json.hpp>

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
    std::unique_ptr<geom::Geometry> read(const std::string& geoJsonText);

    GeoJSONFeatureCollection readFeatures(const std::string& geoJsonText);

private:

    const geom::GeometryFactory& geometryFactory;

    std::unique_ptr<geom::Geometry> readFeatureForGeometry(nlohmann::json& j);

    GeoJSONFeature readFeature(nlohmann::json& j);

    std::map<std::string,GeoJSONValue> readProperties(nlohmann::json& p);

    GeoJSONValue readProperty(nlohmann::json& p);

    std::unique_ptr<geom::Geometry> readFeatureCollectionForGeometry(nlohmann::json& j);

    GeoJSONFeatureCollection readFeatureCollection(nlohmann::json& j);

    std::unique_ptr<geom::Geometry> readGeometry(nlohmann::json& j);

    std::unique_ptr<geom::Point> readPoint(nlohmann::json& j);

    std::unique_ptr<geom::LineString> readLineString(nlohmann::json& j);

    std::unique_ptr<geom::Polygon> readPolygon(nlohmann::json& j);

    std::unique_ptr<geom::MultiPoint> readMultiPoint(nlohmann::json& j);

    std::unique_ptr<geom::MultiLineString> readMultiLineString(nlohmann::json& j);

    std::unique_ptr<geom::MultiPolygon> readMultiPolygon(nlohmann::json& j);

    std::unique_ptr<geom::GeometryCollection> readGeometryCollection(nlohmann::json& j);

};

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_GEOJSONREADER_H
