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
 **********************************************************************
 *
 * Last port: io/WKTReader.java rev. 1.1 (JTS-1.7)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/ParseException.h>
#include <geos/io/OrdinateSet.h>

#include <string>

// Forward declarations
namespace geos {
namespace io {
class StringTokenizer;
}
namespace geom {
class Coordinate;
class CoordinateSequence;
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
 * \class WKTReader
 * \brief WKT parser class; see also WKTWriter.
 */
class GEOS_DLL WKTReader {
public:

    /**
     * \brief Initialize parser with given GeometryFactory.
     *
     * Note that all Geometry objects created by the
     * parser will contain a pointer to the given factory
     * so be sure you'll keep the factory alive for the
     * whole WKTReader and created Geometry life.
     */
    explicit WKTReader(const geom::GeometryFactory& gf)
        : geometryFactory(&gf)
        , precisionModel(gf.getPrecisionModel())
        , fixStructure(false)
        {};

        /** @deprecated in 3.4.0 */
    explicit WKTReader(const geom::GeometryFactory* gf)
        : geometryFactory(gf)
        , precisionModel(gf->getPrecisionModel())
        , fixStructure(false)
        {};

    /**
     * \brief Initialize parser with default GeometryFactory.
     *
     */
    WKTReader()
        : geometryFactory(geom::GeometryFactory::getDefaultInstance())
        , precisionModel(geometryFactory->getPrecisionModel())
        , fixStructure(false)
        {};

    ~WKTReader() {};

    void
    setFixStructure(bool doFixStructure) {
        fixStructure = doFixStructure;
    }

    /// Parse a WKT string returning a Geometry
    template<typename T>
    std::unique_ptr<T> read(const std::string& wkt) const {
        auto g = read(wkt);
        auto gt = dynamic_cast<const T*>(g.get());
        if (!gt) {
            // Can improve this message once there's a good way to get a string repr of T
            throw io::ParseException("Unexpected WKT type");
        }
        return std::unique_ptr<T>(static_cast<T*>(g.release()));
    }

    std::unique_ptr<geom::Geometry> read(const std::string& wellKnownText) const;

protected:
    std::unique_ptr<geom::CoordinateSequence> getCoordinates(io::StringTokenizer* tokenizer, OrdinateSet& ordinates) const;
    static double getNextNumber(io::StringTokenizer* tokenizer);
    static std::string getNextEmptyOrOpener(io::StringTokenizer* tokenizer, OrdinateSet& dim);
    static std::string getNextCloserOrComma(io::StringTokenizer* tokenizer);
    static std::string getNextCloser(io::StringTokenizer* tokenizer);
    static std::string getNextWord(io::StringTokenizer* tokenizer);
    std::unique_ptr<geom::Geometry> readGeometryTaggedText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::Point> readPointText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::LineString> readLineStringText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::LinearRing> readLinearRingText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::MultiPoint> readMultiPointText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::Polygon> readPolygonText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::MultiLineString> readMultiLineStringText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::MultiPolygon> readMultiPolygonText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
    std::unique_ptr<geom::GeometryCollection> readGeometryCollectionText(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const;
private:
    const geom::GeometryFactory* geometryFactory;
    const geom::PrecisionModel* precisionModel;
    bool fixStructure;

    void getPreciseCoordinate(io::StringTokenizer* tokenizer, OrdinateSet& ordinateFlags, geom::CoordinateXYZM&) const;

    static bool isNumberNext(io::StringTokenizer* tokenizer);
    static bool isOpenerNext(io::StringTokenizer* tokenizer);

    static void readOrdinateFlags(const std::string & s, OrdinateSet& ordinateFlags);
    static bool isTypeName(const std::string & type, const std::string & typeName);
};

} // namespace io
} // namespace geos



