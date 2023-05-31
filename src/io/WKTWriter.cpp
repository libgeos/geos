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
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util.h>
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
    level(0),
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
                                    int p_level,
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

    indent(p_level, &writer);
    switch(geometry.getGeometryTypeId()) {
        case GEOS_POINT:      appendPointTaggedText(static_cast<const Point&>(geometry), outputOrdinates, p_level, writer); break;
        case GEOS_LINESTRING: appendLineStringTaggedText(static_cast<const LineString&>(geometry), outputOrdinates, p_level, writer); break;
        case GEOS_LINEARRING: appendLinearRingTaggedText(static_cast<const LinearRing&>(geometry), outputOrdinates, p_level, writer); break;
        case GEOS_POLYGON:    appendPolygonTaggedText(static_cast<const Polygon&>(geometry), outputOrdinates, p_level, writer); break;
        case GEOS_MULTIPOINT: appendMultiPointTaggedText(static_cast<const MultiPoint&>(geometry), outputOrdinates, p_level, writer); break;
        case GEOS_MULTILINESTRING:    appendMultiLineStringTaggedText(static_cast<const MultiLineString&>(geometry), outputOrdinates, p_level, writer); break;
        case GEOS_MULTIPOLYGON:       appendMultiPolygonTaggedText(static_cast<const MultiPolygon&>(geometry), outputOrdinates, p_level, writer); break;
        case GEOS_GEOMETRYCOLLECTION: appendGeometryCollectionTaggedText(static_cast<const GeometryCollection&>(geometry), outputOrdinates, p_level, writer); break;
    }
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
WKTWriter::appendPointTaggedText(const Point& point, OrdinateSet outputOrdinates, int p_level,
                                 Writer& writer) const
{
    writer.write("POINT ");
    appendOrdinateText(outputOrdinates, writer);

    const CoordinateXY* coord = point.getCoordinate();
    if (coord == nullptr) {
        writer.write("EMPTY");
    } else {
        appendSequenceText(*point.getCoordinatesRO(), outputOrdinates, p_level, false, writer);
    }
}

void
WKTWriter::appendLineStringTaggedText(const LineString& lineString, OrdinateSet outputOrdinates, int p_level,
                                      Writer& writer) const
{
    writer.write("LINESTRING ");
    appendOrdinateText(outputOrdinates, writer);
    appendSequenceText(*lineString.getCoordinatesRO(), outputOrdinates, p_level, false, writer);
}

/**
 * Converts a `LinearRing` to \<LinearRing Tagged Text\>
 * format, then appends it to the writer.
 *
 * @param  linearRing  the `LinearRing` to process
 * @param  writer      the output writer to append to
 */
void
WKTWriter::appendLinearRingTaggedText(const LinearRing& linearRing, OrdinateSet outputOrdinates, int p_level, Writer& writer) const
{
    writer.write("LINEARRING ");
    appendOrdinateText(outputOrdinates, writer);
    appendSequenceText(*linearRing.getCoordinatesRO(), outputOrdinates, p_level, false, writer);
}

void
WKTWriter::appendPolygonTaggedText(const Polygon& polygon, OrdinateSet outputOrdinates, int p_level, Writer& writer) const
{
    writer.write("POLYGON ");
    appendOrdinateText(outputOrdinates, writer);
    appendPolygonText(polygon, outputOrdinates, p_level, false, writer);
}

void
WKTWriter::appendMultiPointTaggedText(const MultiPoint& multipoint, OrdinateSet outputOrdinates, int p_level, Writer& writer) const
{
    writer.write("MULTIPOINT ");
    appendOrdinateText(outputOrdinates, writer);
    appendMultiPointText(multipoint, outputOrdinates, p_level, writer);
}

void
WKTWriter::appendMultiLineStringTaggedText(const MultiLineString& multiLineString, OrdinateSet outputOrdinates, int p_level, Writer& writer) const
{
    writer.write("MULTILINESTRING ");
    appendOrdinateText(outputOrdinates, writer);
    appendMultiLineStringText(multiLineString, outputOrdinates, p_level, false, writer);
}

void
WKTWriter::appendMultiPolygonTaggedText(const MultiPolygon& multiPolygon, OrdinateSet outputOrdinates, int p_level, Writer& writer) const
{
    writer.write("MULTIPOLYGON ");
    appendOrdinateText(outputOrdinates, writer);
    appendMultiPolygonText(multiPolygon, outputOrdinates, p_level, writer);
}

void
WKTWriter::appendGeometryCollectionTaggedText(const GeometryCollection& geometryCollection, OrdinateSet outputOrdinates, int p_level,
        Writer& writer) const
{
    writer.write("GEOMETRYCOLLECTION ");
    appendOrdinateText(outputOrdinates, writer);
    appendGeometryCollectionText(geometryCollection, outputOrdinates, p_level, writer);
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
                              int p_level,
                              bool doIndent,
                              Writer& writer) const
{
    if (seq.isEmpty()) {
        writer.write("EMPTY");
    }
    else {
        if(doIndent) {
            indent(p_level, &writer);
        }
        writer.write("(");
        CoordinateXYZM c;
        for(std::size_t i = 0, n = seq.size(); i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                if(coordsPerLine > 0 && i % coordsPerLine == 0) {
                    indent(p_level + 2, &writer);
                }
            }
            seq.getAt(i, c);
            appendCoordinate(c, outputOrdinates, writer);
        }
        writer.write(")");
    }
}

/* protected */
std::string
WKTWriter::writeNumber(double d) const
{
    uint32_t precision = decimalPlaces >= 0 ? static_cast<std::uint32_t>(decimalPlaces) : 0;
    /*
    * For a "trimmed" result, with no trailing zeros we use
    * the ryu library.
    */
    if (trim) {
        char buf[128];
        int len = geos_d2sfixed_buffered_n(d, precision, buf);
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

void
WKTWriter::appendLineStringText(const LineString& lineString, OrdinateSet outputOrdinates, int p_level,
                                bool doIndent, Writer& writer) const
{
    appendSequenceText(*lineString.getCoordinatesRO(), outputOrdinates, p_level, doIndent, writer);
}

void
WKTWriter::appendPolygonText(const Polygon& polygon, OrdinateSet outputOrdinates, int /*level*/,
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
        appendLineStringText(*polygon.getExteriorRing(), outputOrdinates, level, false, writer);
        for(std::size_t i = 0, n = polygon.getNumInteriorRing(); i < n; ++i) {
            writer.write(", ");
            const LineString* ls = polygon.getInteriorRingN(i);
            appendLineStringText(*ls, outputOrdinates, level + 1, true, writer);
        }
        writer.write(")");
    }
}

void
WKTWriter::appendMultiPointText(const MultiPoint& multiPoint, OrdinateSet outputOrdinates,
                                int /*level*/, Writer& writer) const
{
    if(multiPoint.isEmpty()) {
        writer.write("EMPTY");
    }
    else {
        writer.write("(");
        for(std::size_t i = 0, n = multiPoint.getNumGeometries(); i < n; ++i) {
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
WKTWriter::appendMultiLineStringText(const MultiLineString& multiLineString, OrdinateSet outputOrdinates, int p_level, bool indentFirst,
                                     Writer& writer) const
{
    if(multiLineString.isEmpty()) {
        writer.write("EMPTY");
    }
    else {
        int level2 = p_level;
        bool doIndent = indentFirst;
        writer.write("(");
        for(std::size_t i = 0, n = multiLineString.getNumGeometries();
                i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                level2 = p_level + 1;
                doIndent = true;
            }
            const LineString* ls = multiLineString.getGeometryN(i);
            appendLineStringText(*ls, outputOrdinates, level2, doIndent, writer);
        }
        writer.write(")");
    }
}

void
WKTWriter::appendMultiPolygonText(const MultiPolygon& multiPolygon, OrdinateSet outputOrdinates, int p_level, Writer& writer) const
{
    if(multiPolygon.isEmpty()) {
        writer.write("EMPTY");
    }
    else {
        int level2 = p_level;
        bool doIndent = false;
        writer.write("(");
        for(std::size_t i = 0, n = multiPolygon.getNumGeometries();
                i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                level2 = p_level + 1;
                doIndent = true;
            }
            const Polygon* p = multiPolygon.getGeometryN(i);
            appendPolygonText(*p, outputOrdinates, level2, doIndent, writer);
        }
        writer.write(")");
    }
}

void
WKTWriter::appendGeometryCollectionText(
    const GeometryCollection& geometryCollection,
    OrdinateSet outputOrdinates,
    int p_level,
    Writer& writer) const
{
    if(geometryCollection.getNumGeometries() > 0) {
        int level2 = p_level;
        writer.write("(");
        for(std::size_t i = 0, n = geometryCollection.getNumGeometries();
                i < n; ++i) {
            if(i > 0) {
                writer.write(", ");
                level2 = p_level + 1;
            }
            appendGeometryTaggedText(*geometryCollection.getGeometryN(i), outputOrdinates, level2, writer);
        }
        writer.write(")");
    }
    else {
        writer.write("EMPTY");
    }
}

void
WKTWriter::indent(int p_level, Writer* writer) const
{
    if(!isFormatted || p_level <= 0) {
        return;
    }
    writer->write("\n");
    writer->write(std::string(INDENT * static_cast<std::size_t>(p_level), ' '));
}

} // namespace geos.io
} // namespace geos

