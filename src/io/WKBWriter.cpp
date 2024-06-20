/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: io/WKBWriter.java rev. 1.1 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/io/WKBWriter.h>
#include <geos/io/WKBReader.h>
#include <geos/io/ByteOrderValues.h>
#include <geos/io/CheckOrdinatesFilter.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>

#include <ostream>
#include <sstream>
#include <cassert>

#include "geos/util.h"

#undef DEBUG_WKB_WRITER


using namespace geos::geom;

namespace geos {
namespace io { // geos.io

WKBWriter::WKBWriter(uint8_t dims, int bo, bool srid, int flv)
    : defaultOutputDimension(dims)
    , outputOrdinates(getOutputOrdinates(OrdinateSet::createXYZM()))
    , byteOrder(bo)
    , flavor(flv)
    , includeSRID(srid)
    , outStream(nullptr)
{
    if(dims < 2 || dims > 4) {
        throw util::IllegalArgumentException("WKB output dimension must be 2, 3, or 4");
    }
}


/* public */
void
WKBWriter::setOutputDimension(uint8_t dims)
{
    if(dims < 2 || dims > 4) {
        throw util::IllegalArgumentException("WKB output dimension must be 2, 3, or 4");
    }
    defaultOutputDimension = dims;
}


/* public */
void
WKBWriter::setFlavor(int newFlavor)
{
    if (newFlavor != WKBConstants::wkbIso &&
        newFlavor != WKBConstants::wkbExtended) {
        throw util::IllegalArgumentException("Invalid WKB output flavour");
    }
    flavor = newFlavor;
}


void
WKBWriter::writeHEX(const Geometry& g, std::ostream& os)
{
    // setup input/output stream
    std::stringstream stream;

    // write the geometry in wkb format
    this->write(g, stream);

    // convert to HEX
    WKBReader::printHEX(stream, os);
}

void
WKBWriter::write(const Geometry& g, std::ostream& os)
{
    OrdinateSet inputOrdinates = OrdinateSet::createXY();
    inputOrdinates.setM(g.hasM());
    inputOrdinates.setZ(g.hasZ());
    outputOrdinates = getOutputOrdinates(inputOrdinates);

    outStream = &os;

    switch(g.getGeometryTypeId()) {
        case GEOS_POINT: writePoint(static_cast<const Point&>(g)); break;
        case GEOS_LINESTRING:
        case GEOS_LINEARRING:
        case GEOS_CIRCULARSTRING: writeSimpleCurve(static_cast<const SimpleCurve&>(g)); break;
        case GEOS_COMPOUNDCURVE: writeCompoundCurve(static_cast<const CompoundCurve&>(g)); break;
        case GEOS_POLYGON: writePolygon(static_cast<const Polygon&>(g)); break;
        case GEOS_CURVEPOLYGON: writeCurvePolygon(static_cast<const CurvePolygon&>(g)); break;
        case GEOS_MULTIPOINT:
        case GEOS_MULTILINESTRING:
        case GEOS_MULTIPOLYGON:
        case GEOS_MULTICURVE:
        case GEOS_MULTISURFACE:
        case GEOS_GEOMETRYCOLLECTION: writeGeometryCollection(static_cast<const GeometryCollection&>(g)); break;
    }
}

void
WKBWriter::writePointEmpty(const Point& g)
{
    writeByteOrder();
    writeGeometryType(WKBConstants::wkbPoint, g.getSRID());
    writeSRID(g.getSRID());

    Coordinate c(DoubleNotANumber, DoubleNotANumber, DoubleNotANumber);
    CoordinateSequence cas(std::size_t(1), std::size_t(g.getCoordinateDimension()));
    cas.setAt(c, 0);

    writeCoordinateSequence(cas, false);
}

void
WKBWriter::writePoint(const Point& g)
{
    if (g.isEmpty()) {
        return writePointEmpty(g);
    }

    writeByteOrder();

    writeGeometryType(WKBConstants::wkbPoint, g.getSRID());
    writeSRID(g.getSRID());

    const CoordinateSequence* cs = g.getCoordinatesRO();
    assert(cs);
    writeCoordinateSequence(*cs, false);
}

void
WKBWriter::writeSimpleCurve(const SimpleCurve& g)
{
    writeByteOrder();

    writeGeometryType(getWkbType(g), g.getSRID());
    writeSRID(g.getSRID());

    const CoordinateSequence* cs = g.getCoordinatesRO();
    assert(cs);
    writeCoordinateSequence(*cs, true);
}

void
WKBWriter::writeCompoundCurve(const CompoundCurve& g)
{
    writeByteOrder();

    writeGeometryType(getWkbType(g), g.getSRID());
    writeSRID(g.getSRID());

    writeInt(static_cast<int>(g.getNumCurves()));

    auto orig_includeSRID = includeSRID;
    includeSRID = false;

    for (std::size_t i = 0; i < g.getNumCurves(); i++) {
        const SimpleCurve& section = *g.getCurveN(i);
        writeSimpleCurve(section);
    }

    includeSRID = orig_includeSRID;
}

void
WKBWriter::writePolygon(const Polygon& g)
{
    writeByteOrder();

    writeGeometryType(getWkbType(g), g.getSRID());
    writeSRID(g.getSRID());

    if (g.isEmpty()) {
        writeInt(0);
        return;
    }

    std::size_t nholes = g.getNumInteriorRing();
    writeInt(static_cast<int>(nholes + 1));

    const LineString* ls = g.getExteriorRing();
    assert(ls);

    const CoordinateSequence* cs = ls->getCoordinatesRO();
    assert(cs);

    writeCoordinateSequence(*cs, true);
    for(std::size_t i = 0; i < nholes; i++) {
        ls = g.getInteriorRingN(i);
        assert(ls);

        cs = ls->getCoordinatesRO();
        assert(cs);

        writeCoordinateSequence(*cs, true);
    }
}

void
WKBWriter::writeCurvePolygon(const CurvePolygon& g)
{
    // Why not combine this with writePolygon?
    // A CurvePolygon differs from a Polygon in that its rings
    // can one of three different types. Therefore, the type
    // information is written for each ring, unlike a Polygon.

    writeByteOrder();

    writeGeometryType(getWkbType(g), g.getSRID());

    writeSRID(g.getSRID());

    if (g.isEmpty()) {
        writeInt(0);
        return;
    }

    std::size_t nholes = g.getNumInteriorRing();
    writeInt(static_cast<int>(nholes + 1));

    const Curve* ring = g.getExteriorRing();
    write(*ring, *outStream);

    for(std::size_t i = 0; i < nholes; i++) {
        ring = g.getInteriorRingN(i);
        write(*ring, *outStream);
    }
}

void
WKBWriter::writeGeometryCollection(const GeometryCollection& g)
{
    writeByteOrder();

    writeGeometryType(getWkbType(g), g.getSRID());
    writeSRID(g.getSRID());

    auto ngeoms = g.getNumGeometries();
    writeInt(static_cast<int>(ngeoms));
    auto orig_includeSRID = includeSRID;
    includeSRID = false;

    assert(outStream);
    for(std::size_t i = 0; i < ngeoms; i++) {
        const Geometry* elem = g.getGeometryN(i);
        assert(elem);

        write(*elem, *outStream);
    }
    includeSRID = orig_includeSRID;
}

void
WKBWriter::writeByteOrder()
{
    if(byteOrder == ByteOrderValues::ENDIAN_LITTLE) {
        buf[0] = WKBConstants::wkbNDR;
    }
    else {
        buf[0] = WKBConstants::wkbXDR;
    }

    assert(outStream);
    outStream->write(reinterpret_cast<char*>(buf), 1);
}

/* public */
void
WKBWriter::setByteOrder(int bo)
{
    if(bo != ByteOrderValues::ENDIAN_LITTLE &&
            bo != ByteOrderValues::ENDIAN_BIG) {
        std::ostringstream os;
        os << "WKB output dimension must be LITTLE ("
           << ByteOrderValues::ENDIAN_LITTLE
           << ") or BIG (" << ByteOrderValues::ENDIAN_BIG << ")";
        throw util::IllegalArgumentException(os.str());
    }

    byteOrder = bo;
}

void
WKBWriter::writeGeometryType(int typeId, int SRID)
{
    if (flavor == WKBConstants::wkbExtended) {
        int dimFlag = 0;
        if (outputOrdinates.hasZ()) {
            dimFlag |= static_cast<int>(0x80000000);
        }
        if (outputOrdinates.hasM()) {
            dimFlag |= static_cast<int>(0x40000000);
        }

        typeId |= dimFlag;
        if(includeSRID && SRID != 0) {
            typeId |= 0x20000000;
        }
    }
    else if (flavor == WKBConstants::wkbIso) {
        if (outputOrdinates.hasZ()) {
            typeId += 1000;
        }
        if (outputOrdinates.hasM()) {
            typeId += 2000;
        }
    }
    else {
        throw util::IllegalArgumentException("Unknown WKB flavor");
    }
    writeInt(typeId);
}

void
WKBWriter::writeSRID(int SRID)
{
    // Only write the SRID in if
    // it is requested and
    // it is non zero and
    // the format is extended (ISO doesn't support SRID embedding)
    if (includeSRID &&
        SRID != 0 &&
        flavor == WKBConstants::wkbExtended)
    {
        writeInt(SRID);
    }
}

void
WKBWriter::writeInt(int val)
{
    ByteOrderValues::putInt(val, buf, byteOrder);
    outStream->write(reinterpret_cast<char*>(buf), 4);
    //outStream->write(reinterpret_cast<char *>(&val), 4);
}

void
WKBWriter::writeCoordinateSequence(const CoordinateSequence& cs,
                                   bool sized)
{
    std::size_t size = cs.getSize();

    if(sized) {
        writeInt(static_cast<int>(size));
    }
    for(std::size_t i = 0; i < size; i++) {
        writeCoordinate(cs, i);
    }
}

void
WKBWriter::writeCoordinate(const CoordinateSequence& cs, std::size_t idx)
{
#if DEBUG_WKB_WRITER
    std::size_t << "writeCoordinate: X:" << cs.getX(idx) << " Y:" << cs.getY(idx) << std::endl;
#endif
    assert(outStream);

    CoordinateXYZM coord(DoubleNotANumber, DoubleNotANumber, DoubleNotANumber, DoubleNotANumber);
    cs.getAt(idx, coord);

    ByteOrderValues::putDouble(coord.x, buf, byteOrder);
    outStream->write(reinterpret_cast<char*>(buf), 8);
    ByteOrderValues::putDouble(coord.y, buf, byteOrder);
    outStream->write(reinterpret_cast<char*>(buf), 8);
    if(outputOrdinates.hasZ()) {
        ByteOrderValues::putDouble(coord.z, buf, byteOrder);
        outStream->write(reinterpret_cast<char*>(buf), 8);
    }
    if(outputOrdinates.hasM()) {
        ByteOrderValues::putDouble(coord.m, buf, byteOrder);
        outStream->write(reinterpret_cast<char*>(buf), 8);
    }
}

OrdinateSet
WKBWriter::getOutputOrdinates(OrdinateSet ordinates)
{
    // drop specified ordinates to meet defaultOutputDimension
    OrdinateSet newOrdinates = ordinates;
    while (newOrdinates.size() > defaultOutputDimension) {
        if (newOrdinates.hasM()) {
            newOrdinates.setM(false);
        } else if (newOrdinates.hasZ()) {
            newOrdinates.setZ(false);
        }
    }

    return newOrdinates;
}

int
WKBWriter::getWkbType(const Geometry& g) {
    switch(g.getGeometryTypeId()) {
        case GEOS_POINT: return WKBConstants::wkbPoint;
        case GEOS_LINESTRING:
        case GEOS_LINEARRING: return WKBConstants::wkbLineString;
        case GEOS_CIRCULARSTRING: return WKBConstants::wkbCircularString;
        case GEOS_COMPOUNDCURVE: return WKBConstants::wkbCompoundCurve;
        case GEOS_POLYGON: return WKBConstants::wkbPolygon;
        case GEOS_CURVEPOLYGON: return WKBConstants::wkbCurvePolygon;
        case GEOS_MULTIPOINT: return WKBConstants::wkbMultiPoint;
        case GEOS_MULTILINESTRING: return WKBConstants::wkbMultiLineString;
        case GEOS_MULTICURVE: return WKBConstants::wkbMultiCurve;
        case GEOS_MULTIPOLYGON: return WKBConstants::wkbMultiPolygon;
        case GEOS_MULTISURFACE: return WKBConstants::wkbMultiSurface;
        case GEOS_GEOMETRYCOLLECTION: return WKBConstants::wkbGeometryCollection;
    }

    // Avoid -Wreturn-type warning
    throw util::IllegalArgumentException("Invalid geometry type.");
}


} // namespace geos.io
} // namespace geos

