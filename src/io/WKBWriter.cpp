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

    if (const Point* x = dynamic_cast<const Point*>(&g)) {
        return writePoint(*x);
    }

    if (const LineString* x = dynamic_cast<const LineString*>(&g)) {
        return writeLineString(*x);
    }

    if (const Polygon* x = dynamic_cast<const Polygon*>(&g)) {
        return writePolygon(*x);
    }

    if (const MultiPoint* x = dynamic_cast<const MultiPoint*>(&g)) {
        return writeGeometryCollection(*x, WKBConstants::wkbMultiPoint);
    }

    if (const MultiLineString* x = dynamic_cast<const MultiLineString*>(&g)) {
        return writeGeometryCollection(*x, WKBConstants::wkbMultiLineString);
    }

    if (const MultiPolygon* x = dynamic_cast<const MultiPolygon*>(&g)) {
        return writeGeometryCollection(*x, WKBConstants::wkbMultiPolygon);
    }

    if (const GeometryCollection* x =
                dynamic_cast<const GeometryCollection*>(&g)) {
        return writeGeometryCollection(*x, WKBConstants::wkbGeometryCollection);
    }

    assert(0); // Unknown Geometry type
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
WKBWriter::writeLineString(const LineString& g)
{
    writeByteOrder();

    writeGeometryType(WKBConstants::wkbLineString, g.getSRID());
    writeSRID(g.getSRID());

    const CoordinateSequence* cs = g.getCoordinatesRO();
    assert(cs);
    writeCoordinateSequence(*cs, true);
}

void
WKBWriter::writePolygon(const Polygon& g)
{
    writeByteOrder();

    writeGeometryType(WKBConstants::wkbPolygon, g.getSRID());
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
WKBWriter::writeGeometryCollection(const GeometryCollection& g,
                                   int wkbtype)
{
    writeByteOrder();

    writeGeometryType(wkbtype, g.getSRID());
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


} // namespace geos.io
} // namespace geos

