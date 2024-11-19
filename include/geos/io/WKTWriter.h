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
 **********************************************************************
 *
 * Last port: io/WKTWriter.java rev. 1.34 (JTS-1.7)
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/io/OrdinateSet.h>

#include <string>
#include <cctype>
#include <cstdint>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateXY;
class CoordinateXYZM;
class CoordinateSequence;
class Curve;
class CompoundCurve;
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
class SimpleCurve;
class Surface;
}
namespace io {
class Writer;
}
}


namespace geos {
namespace io {

/**
 * \class WKTWriter
 *
 * \brief Outputs the textual representation of a Geometry.
 * See also WKTReader.
 *
 * The WKTWriter outputs coordinates rounded to the precision
 * model. No more than the maximum number of necessary decimal places will be
 * output.
 *
 * The Well-known Text format is defined in the <A
 * HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 * Specification for SQL</A>.
 *
 * A non-standard "LINEARRING" tag is used for LinearRings. The WKT spec does
 * not define a special tag for LinearRings. The standard tag to use is
 * "LINESTRING".
 *
 * See WKTReader for parsing.
 *
 */
class GEOS_DLL WKTWriter {
public:
    WKTWriter();
    ~WKTWriter() = default;

    //string(count, ch) can be used for this
    //static string stringOfChar(char ch, int count);

    /// Returns WKT string for the given Geometry
    std::string write(const geom::Geometry* geometry);

    std::string write(const geom::Geometry& geometry);

    // Send Geometry's WKT to the given Writer
    void write(const geom::Geometry* geometry, Writer* writer);

    std::string writeFormatted(const geom::Geometry* geometry);

    void writeFormatted(const geom::Geometry* geometry, Writer* writer);

    /**
     * Generates the WKT for a N-point <code>LineString</code>.
     *
     * @param seq the sequence to output
     *
     * @return the WKT
     */
    static std::string toLineString(const geom::CoordinateSequence& seq);

    /**
     * Generates the WKT for a 2-point <code>LineString</code>.
     *
     * @param p0 the first coordinate
     * @param p1 the second coordinate
     *
     * @return the WKT
     */
    static std::string toLineString(const geom::CoordinateXY& p0, const geom::CoordinateXY& p1);

    /**
     * Generates the WKT for a <code>Point</code>.
     *
     * @param p0 the point coordinate
     *
     * @return the WKT
     */
    static std::string toPoint(const geom::Coordinate& p0);
    static std::string toPoint(const geom::CoordinateXY& p0);

    /**
     * Sets the rounding precision when writing the WKT
     * a precision of -1 disables it
     *
     * @param p0 the new precision to use
     *
     */
    void setRoundingPrecision(int p0);

    /**
     * Enables/disables trimming of unnecessary decimals
     *
     * @param p0 the trim boolean
     *
     */
    void setTrim(bool p0);

    /**
     * Enables/disables removal of Z/M dimensions that have
     * no non-NaN values in a geometry.
     *
     * @brief setRemoveEmptyDimensions
     * @param remove
     */
    void setRemoveEmptyDimensions(bool remove)
    {
        removeEmptyDimensions = remove;
    }

    /**
     * Enable old style 3D/4D WKT generation.
     *
     * By default the WKTWriter produces new style 3D/4D WKT
     * (ie. "POINT Z (10 20 30)") but if this method is used
     * to turn on old style WKT production then the WKT will
     * be formatted in the style "POINT (10 20 30)".
     *
     * @param useOld3D true or false
     */
    void
    setOld3D(bool useOld3D)
    {
        old3D = useOld3D;
    }

    /*
     * \brief
     * Returns the output dimension used by the
     * <code>WKTWriter</code>.
     */
    int
    getOutputDimension() const
    {
        return defaultOutputDimension;
    }

    /*
     * Sets the output dimension used by the <code>WKTWriter</code>.
     *
     * @param newOutputDimension Supported values are 2, 3 or 4.
     *        Default since GEOS 3.12 is 4.
     *        Note that 3 indicates up to 3 dimensions will be
     *        written but 2D WKT is still produced for 2D geometries.
     */
    void setOutputDimension(uint8_t newOutputDimension);

    static std::string writeNumber(double d, bool trim, uint32_t precision);
    static int writeTrimmedNumber(double d, uint32_t precision, char* buf);

protected:

    int decimalPlaces;

    void appendGeometryTaggedText(
            const geom::Geometry& geometry,
            OrdinateSet outputOrdinates,
            int level,
            Writer& writer) const;

    void appendTag(
            const geom::Geometry& geometry,
            OrdinateSet outputOrdinates,
            Writer& writer) const;

    void appendPointTaggedText(
        const geom::Point& point,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendSimpleCurveTaggedText(
        const geom::SimpleCurve& lineString,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendCompoundCurveTaggedText(
        const geom::CompoundCurve& lineString,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendSurfaceTaggedText(
        const geom::Surface& polygon,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendMultiPointTaggedText(
        const geom::MultiPoint& multipoint,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendMultiCurveTaggedText(
        const geom::GeometryCollection& multiCurve,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendMultiSurfaceTaggedText(
        const geom::GeometryCollection& multiSurface,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendGeometryCollectionTaggedText(
        const geom::GeometryCollection& geometryCollection,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendOrdinateText(OrdinateSet outputOrdinates,
                            Writer& writer) const;

    void appendSequenceText(const geom::CoordinateSequence& seq,
                            OrdinateSet outputOrdinates,
                            int level,
                            bool doIntent,
                            Writer& writer) const;

    void appendCoordinate(const geom::CoordinateXYZM& coordinate,
                          OrdinateSet outputOrdinates,
                          Writer& writer) const;

    std::string writeNumber(double d) const;

    void appendCurveText(
        const geom::Curve& lineString,
        OrdinateSet outputOrdinates,
        int level, bool doIndent, Writer& writer) const;

    void appendSimpleCurveText(
        const geom::SimpleCurve& lineString,
        OrdinateSet outputOrdinates,
        int level, bool doIndent, Writer& writer) const;

    void appendSurfaceText(
        const geom::Surface& polygon,
        OrdinateSet outputOrdinates,
        int level, bool indentFirst, Writer& writer) const;

    void appendMultiPointText(
        const geom::MultiPoint& multiPoint,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendMultiCurveText(
        const geom::GeometryCollection& multiCurve,
        OrdinateSet outputOrdinates,
        int level, bool indentFirst, Writer& writer) const;

    void appendMultiSurfaceText(
        const geom::GeometryCollection& multiSurface,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

    void appendGeometryCollectionText(
        const geom::GeometryCollection& geometryCollection,
        OrdinateSet outputOrdinates,
        int level, Writer& writer) const;

private:

    enum {
        INDENT = 2
    };

    bool isFormatted;

    int roundingPrecision;

    bool trim;

    bool removeEmptyDimensions = false;

    static constexpr int coordsPerLine = 10;

    uint8_t defaultOutputDimension;
    bool old3D;

    void writeFormatted(
        const geom::Geometry* geometry,
        bool isFormatted, Writer* writer);

    void indent(int level, Writer* writer) const;
};

} // namespace geos::io
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

