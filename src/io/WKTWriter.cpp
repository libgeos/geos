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

#include <geos/io/WKTWriter.h>
#include <geos/io/Writer.h>
#include <geos/io/CLocalizer.h>
#include <geos/io/CheckOrdinatesFilter.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/MultiSurface.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util.h>
#include <geos/util/string.h>
#include <geos/util/IllegalArgumentException.h>

#include <ryu/ryu.h>

#include <algorithm> // for min
#include <typeinfo>
#include <cstdio> // should avoid this
#include <string>
#include <sstream>
#include <cassert>
#include <cmath>
#include <iomanip>


using namespace geos::geom;

namespace geos {
namespace io { // geos.io

WKTWriter::WKTWriter():
    decimalPlaces(6),
    isFormatted(false),
    roundingPrecision(-1),
    trim(true),
    defaultOutputDimension(4),
    old3D(false)
{
}

/* public */
void
WKTWriter::setOutputDimension(uint8_t dims)
{
    if(dims < 2 || dims > 4) {
        throw util::IllegalArgumentException("WKT output dimension must be 2, 3, or 4");
    }
    defaultOutputDimension = dims;
}

/*static*/
std::string
WKTWriter::toLineString(const CoordinateSequence& seq)
{
    std::stringstream buf(std::ios_base::in | std::ios_base::out);
    buf << "LINESTRING ";
    auto npts = seq.size();
    if(npts == 0) {
        buf << "EMPTY";
    }
    else {
        buf << "(";
        for(std::size_t i = 0; i < npts; ++i) {
            if(i) {
                buf << ", ";
            }
            buf << seq.getX(i) << " " << seq.getY(i);
#if PRINT_Z
            buf << seq.getZ(i);
#endif
        }
        buf << ")";
    }

    return buf.str();
}

/*static*/
std::string
WKTWriter::toLineString(const Coordinate& p0, const Coordinate& p1)
{
    std::stringstream ret(std::ios_base::in | std::ios_base::out);
    ret << "LINESTRING (" << p0.x << " " << p0.y;
#if PRINT_Z
    ret << " " << p0.z;
#endif
    ret << ", " << p1.x << " " << p1.y;
#if PRINT_Z
    ret << " " << p1.z;
#endif
    ret << ")";

    return ret.str();
}

/*static*/
std::string
WKTWriter::toPoint(const Coordinate& p0)
{
    std::stringstream ret(std::ios_base::in | std::ios_base::out);
    ret << "POINT (";
#if PRINT_Z
    ret << p0.x << " " << p0.y  << " " << p0.z << " )";
#else
    ret << p0.x << " " << p0.y  << " )";
#endif
    return ret.str();
}

std::string
WKTWriter::toPoint(const CoordinateXY& p0)
{
    std::stringstream ret(std::ios_base::in | std::ios_base::out);
    ret << "POINT (";
    ret << p0.x << " " << p0.y  << " )";
    return ret.str();
}

void
WKTWriter::setRoundingPrecision(int p0)
{
    if(p0 < -1) {
        p0 = -1;
    }
    roundingPrecision = p0;
}

void
WKTWriter::setTrim(bool p0)
{
    trim = p0;
}

std::string
WKTWriter::write(const Geometry* geometry)
{
    Writer sw;
    writeFormatted(geometry, false, &sw);
    std::string res = sw.toString();
    return res;
}

std::string
WKTWriter::write(const Geometry& geometry)
{
    return write(&geometry);
}

void
WKTWriter::write(const Geometry* geometry, Writer* writer)
{
    writeFormatted(geometry, false, writer);
}

std::string
WKTWriter::writeFormatted(const Geometry* geometry)
{
    Writer sw;
    writeFormatted(geometry, true, &sw);
    return sw.toString();
}

void
WKTWriter::writeFormatted(const Geometry* geometry, Writer* writer)
{
    writeFormatted(geometry, true, writer);
}

void
WKTWriter::writeFormatted(const Geometry* geometry, bool p_isFormatted,
                          Writer* writer)
{
    CLocalizer clocale;
    this->isFormatted = p_isFormatted;
    decimalPlaces = roundingPrecision == -1
                    ? geometry->getPrecisionModel()->getMaximumSignificantDigits()
                    : roundingPrecision;

    appendGeometryTaggedText(*geometry, OrdinateSet::createXYZM(), 0, *writer);
}

void
WKTWriter::appendGeometryTaggedText(const Geometry& geometry,
                                    OrdinateSet checkOrdinates,
                                    int level,
                                    Writer& writer) const
{
    OrdinateSet outputOrdinates = OrdinateSet::createXY();
    if (geometry.isEmpty() || !removeEmptyDimensions) {
        // for an empty geometry, use the declared dimensionality
        outputOrdinates.setZ(geometry.hasZ());
        outputOrdinates.setM(geometry.hasM());
    } else {
        // for a non-empty geometry, evaluate the ordinates actually present in the geometry
        CheckOrdinatesFilter cof(checkOrdinates);
        geometry.apply_ro(cof);
        // remove detected ordinates to stay within defaultOutputDimension
        outputOrdinates = cof.getFoundOrdinates();
    }

    while (outputOrdinates.size() > defaultOutputDimension) {
        if (outputOrdinates.hasZ() && outputOrdinates.hasM()) {
            // 4D -> 3D
            outputOrdinates.setM(false);
        } else {
            // 3D -> 2D
            outputOrdinates.setM(false);
            outputOrdinates.setZ(false);
        }
    }

    indent(level, &writer);
    switch(geometry.getGeometryTypeId()) {
        case GEOS_POINT:      appendPointTaggedText(static_cast<const Point&>(geometry), outputOrdinates, level, writer); break;
        case GEOS_LINESTRING:
        case GEOS_LINEARRING:
        case GEOS_CIRCULARSTRING: appendSimpleCurveTaggedText(static_cast<const SimpleCurve&>(geometry), outputOrdinates, level, writer); break;
        case GEOS_COMPOUNDCURVE: appendCompoundCurveTaggedText(static_cast<const CompoundCurve&>(geometry), outputOrdinates, level, writer); break;
        case GEOS_CURVEPOLYGON:
        case GEOS_POLYGON:    appendSurfaceTaggedText(static_cast<const Polygon&>(geometry), outputOrdinates, level, writer); break;
        case GEOS_MULTIPOINT: appendMultiPointTaggedText(static_cast<const MultiPoint&>(geometry), outputOrdinates, level, writer); break;
        case GEOS_MULTICURVE:
        case GEOS_MULTILINESTRING:    appendMultiCurveTaggedText(static_cast<const MultiLineString&>(geometry), outputOrdinates, level, writer); break;
        case GEOS_MULTISURFACE:
        case GEOS_MULTIPOLYGON:       appendMultiSurfaceTaggedText(static_cast<const MultiPolygon&>(geometry), outputOrdinates, level, writer); break;
        case GEOS_GEOMETRYCOLLECTION: appendGeometryCollectionTaggedText(static_cast<const GeometryCollection&>(geometry), outputOrdinates, level, writer); break;
    }
}

void WKTWriter::appendTag(const Geometry& geometry, OrdinateSet outputOrdinates, Writer& writer) const
{
    std::string type = geometry.getGeometryType();
    util::toUpper(type);
    writer.write(type);
    writer.write(" ");
    appendOrdinateText(outputOrdinates, writer);
}

/*protected*/
void
WKTWriter::appendOrdinateText(OrdinateSet outputOrdinates, Writer& writer) const
{
    if (old3D) {
        if (!outputOrdinates.hasZ() && outputOrdinates.hasM()) {
            writer.write("M ");
        }
        return;
    }

    bool writeSpace = false;
    if (outputOrdinates.hasZ()) {
        writer.write("Z");
        writeSpace = true;
    }
    if (outputOrdinates.hasM()) {
        writer.write("M");
        writeSpace = true;
    }
    if (writeSpace) {
        writer.write(" ");
    }
}

void
WKTWriter::appendPointTaggedText(const Point& point, OrdinateSet outputOrdinates, int level,
                                 Writer& writer) const
{
    writer.write("POINT ");
    appendOrdinateText(outputOrdinates, writer);

    const CoordinateXY* coord = point.getCoordinate();
    if (coord == nullptr) {
        writer.write("EMPTY");
    } else {
        appendSequenceText(*point.getCoordinatesRO(), outputOrdinates, level, false, writer);
    }
}

void
WKTWriter::appendSimpleCurveTaggedText(const SimpleCurve& curve, OrdinateSet outputOrdinates, int level, Writer& writer) const
{
    appendTag(curve, outputOrdinates, writer);
    appendSequenceText(*curve.getCoordinatesRO(), outputOrdinates, level, false, writer);
}

void
WKTWriter::appendCurveText(const Curve& curve, OrdinateSet outputOrdinates, int level, bool doIndent, Writer& writer) const {
    if (doIndent) {
        indent(level, &writer);
    }

    if (curve.getGeometryTypeId() == GEOS_COMPOUNDCURVE) {
        appendCompoundCurveTaggedText(static_cast<const CompoundCurve&>(curve), outputOrdinates, level, writer);
    } else {
        appendSimpleCurveText(static_cast<const SimpleCurve&>(curve), outputOrdinates, level, false, writer);
    }
}

void
WKTWriter::appendSimpleCurveText(const SimpleCurve& curve, OrdinateSet outputOrdinates, int level, bool doIndent, Writer& writer) const {
    if (doIndent) {
        indent(level, &writer);
    }

    if (curve.getGeometryTypeId() == GEOS_CIRCULARSTRING) {
        appendSimpleCurveTaggedText(curve, outputOrdinates, level, writer);
    } else {
        appendSequenceText(*curve.getCoordinatesRO(), outputOrdinates, level, false, writer);
    }
}


void
WKTWriter::appendCompoundCurveTaggedText(const CompoundCurve& curve, OrdinateSet outputOrdinates, int level, Writer& writer) const
{
    writer.write("COMPOUNDCURVE ");
    appendOrdinateText(outputOrdinates, writer);

    if (curve.isEmpty()) {
        writer.write("EMPTY");
    } else {
        writer.write("(");
        bool indentFirst = false;
        for (std::size_t i = 0; i < curve.getNumCurves(); i++) {
            if (i > 0) {
                writer.write(", ");
                indentFirst = true;
            }

            appendSimpleCurveText(*curve.getCurveN(i), outputOrdinates, level + (i > 0), indentFirst, writer);
        }
        writer.write(")");
    }
}

void
WKTWriter::appendSurfaceTaggedText(const Surface& surface, OrdinateSet outputOrdinates, int level, Writer& writer) const
{
    appendTag(surface, outputOrdinates, writer);
    appendSurfaceText(surface, outputOrdinates, level, false, writer);
}

void
WKTWriter::appendMultiPointTaggedText(const MultiPoint& multipoint, OrdinateSet outputOrdinates, int level, Writer& writer) const
{
    writer.write("MULTIPOINT ");
    appendOrdinateText(outputOrdinates, writer);
    appendMultiPointText(multipoint, outputOrdinates, level, writer);
}

void
WKTWriter::appendMultiCurveTaggedText(const GeometryCollection& multiCurve, OrdinateSet outputOrdinates, int level, Writer& writer) const
{
    appendTag(multiCurve, outputOrdinates, writer);
    appendMultiCurveText(multiCurve, outputOrdinates, level, false, writer);
}

void
WKTWriter::appendMultiSurfaceTaggedText(const GeometryCollection& multiPolygon, OrdinateSet outputOrdinates, int level, Writer& writer) const
{
    appendTag(multiPolygon, outputOrdinates, writer);
    appendMultiSurfaceText(multiPolygon, outputOrdinates, level, writer);
}

void
WKTWriter::appendGeometryCollectionTaggedText(const GeometryCollection& geometryCollection, OrdinateSet outputOrdinates, int level,
        Writer& writer) const
{
    writer.write("GEOMETRYCOLLECTION ");
    appendOrdinateText(outputOrdinates, writer);
    appendGeometryCollectionText(geometryCollection, outputOrdinates, level, writer);
}

/* protected */
void
WKTWriter::appendCoordinate(const CoordinateXYZM& coordinate,
                            OrdinateSet outputOrdinates,
                            Writer& writer) const
{
    writer.write(writeNumber(coordinate.x));
    writer.write(" ");
    writer.write(writeNumber(coordinate.y));

    if(outputOrdinates.hasZ()) {
        writer.write(" ");
        writer.write(writeNumber(coordinate.z));
    }

    if(outputOrdinates.hasM()) {
        writer.write(" ");
        writer.write(writeNumber(coordinate.m));
    }
}

void
WKTWriter::appendSequenceText(const CoordinateSequence& seq,
                              OrdinateSet outputOrdinates,
                              int level,
                              bool doIndent,
                              Writer& writer) const
{
    if (seq.isEmpty()) {
        writer.write("EMPTY");
    }
    else {
        if(doIndent) {
            indent(level, &writer);
        }
        writer.write("(");
        CoordinateXYZM c;
        for(std::size_t i = 0, n = seq.size(); i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                if(coordsPerLine > 0 && i % coordsPerLine == 0) {
                    indent(level + 2, &writer);
                }
            }
            seq.getAt(i, c);
            appendCoordinate(c, outputOrdinates, writer);
        }
        writer.write(")");
    }
}

int
WKTWriter::writeTrimmedNumber(double d, uint32_t precision, char* buf)
{
    const auto da = std::fabs(d);
    if ( !std::isfinite(d) || (da == 0.0) )
        // non-finite or exactly zero
        return geos_d2sfixed_buffered_n(d, precision, buf);
    else if ( (da >= 1e+17) || (da < 1e-4) )
        // very large or small numbers, use scientific notation
        return geos_d2sexp_buffered_n(d, precision, buf);
    else {
        // most real-world coordinates, use positional notation
        if ( (precision < 4) && (da < 1.0) ) {
            // adjust precision to avoid rounding to zero
            precision = static_cast<std::uint32_t>(-floor(log10(da)));
        }
        return geos_d2sfixed_buffered_n(d, precision, buf);
    }
}

std::string
WKTWriter::writeNumber(double d, bool trim, uint32_t precision) {
    /*
    * For a "trimmed" result, with no trailing zeros we use
    * the ryu library.
    */
    if (trim) {
        char buf[28];
        int len = writeTrimmedNumber(d, precision, buf);
        buf[len] = '\0';
        std::string s(buf);
        return s;
    }
    /*
    * For an "untrimmed" result, compatible with the old
    * format, we continue to use std::fixed.
    */
    else {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(static_cast<int>(precision));
        ss << d;
        return ss.str();
    }
}

/* protected */
std::string
WKTWriter::writeNumber(double d) const
{
    uint32_t precision = decimalPlaces >= 0 ? static_cast<std::uint32_t>(decimalPlaces) : 0;
    return writeNumber(d, trim, precision);
}

void
WKTWriter::appendSurfaceText(const Surface& polygon, OrdinateSet outputOrdinates, int level,
                             bool indentFirst, Writer& writer) const
{
    if(polygon.isEmpty()) {
        writer.write("EMPTY");
    }
    else {
        if(indentFirst) {
            indent(level, &writer);
        }
        writer.write("(");

        auto ring = polygon.getExteriorRing();
        appendCurveText(*ring, outputOrdinates, level, false, writer);

        for(std::size_t i = 0, n = polygon.getNumInteriorRing(); i < n; ++i) {
            writer.write(", ");

            auto hole = polygon.getInteriorRingN(i);
            appendCurveText(*hole, outputOrdinates, level + 1, true, writer);
        }
        writer.write(")");
    }
}

void
WKTWriter::appendMultiPointText(const MultiPoint& multiPoint, OrdinateSet outputOrdinates,
                                int /*level*/, Writer& writer) const
{
    const std::size_t n = multiPoint.getNumGeometries();
    if(n == 0) {
        writer.write("EMPTY");
    }
    else {
        writer.write("(");
        for(std::size_t i = 0; i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
            }
            const CoordinateSequence* seq = multiPoint.getGeometryN(i)->getCoordinatesRO();
            if(seq == nullptr || seq->isEmpty()) {
                writer.write("EMPTY");
            }
            else {
                CoordinateXYZM coord;
                writer.write("(");
                seq->getAt(0, coord);
                appendCoordinate(coord, outputOrdinates, writer);
                writer.write(")");
            }
        }
        writer.write(")");
    }
}

void
WKTWriter::appendMultiCurveText(const GeometryCollection& multiCurve, OrdinateSet outputOrdinates, int level, bool indentFirst,
                                     Writer& writer) const
{
    const std::size_t n = multiCurve.getNumGeometries();
    if(n == 0) {
        writer.write("EMPTY");
    }
    else {
        int level2 = level;
        bool doIndent = indentFirst;
        writer.write("(");
        for(std::size_t i = 0; i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                level2 = level + 1;
                doIndent = true;
            }

            const Curve* g = static_cast<const Curve*>(multiCurve.getGeometryN(i));
            appendCurveText(*g, outputOrdinates, level2, doIndent, writer);
        }
        writer.write(")");
    }
}

void
WKTWriter::appendMultiSurfaceText(const GeometryCollection& multiSurface, OrdinateSet outputOrdinates, int level, Writer& writer) const
{
    const std::size_t n = multiSurface.getNumGeometries();
    if(n == 0) {
        writer.write("EMPTY");
    }
    else {
        int level2 = level;
        bool doIndent = false;
        writer.write("(");
        for(std::size_t i = 0; i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                level2 = level + 1;
                doIndent = true;
            }
            const Surface* p = static_cast<const Surface*>(multiSurface.getGeometryN(i));
            if (p->getGeometryTypeId() == GEOS_POLYGON) {
                appendSurfaceText(*p, outputOrdinates, level2, doIndent, writer);
            } else {
                // FIXME indent
                appendSurfaceTaggedText(*p, outputOrdinates, level2, writer);
            }
        }
        writer.write(")");
    }
}

void
WKTWriter::appendGeometryCollectionText(
    const GeometryCollection& geometryCollection,
    OrdinateSet outputOrdinates,
    int level,
    Writer& writer) const
{
    const std::size_t n = geometryCollection.getNumGeometries();
    if(n == 0) {
        writer.write("EMPTY");
    }
    else {
        int level2 = level;
        writer.write("(");
        for(std::size_t i = 0; i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                level2 = level + 1;
            }
            appendGeometryTaggedText(*geometryCollection.getGeometryN(i), outputOrdinates, level2, writer);
        }
        writer.write(")");
    }
}

void
WKTWriter::indent(int level, Writer* writer) const
{
    if(!isFormatted || level <= 0) {
        return;
    }
    writer->write("\n");
    writer->write(std::string(INDENT * static_cast<std::size_t>(level), ' '));
}

} // namespace geos.io
} // namespace geos

