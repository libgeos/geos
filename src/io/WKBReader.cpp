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
 * Last port: io/WKBReader.java rev. 1.1 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/io/WKBReader.h>
#include <geos/io/WKBConstants.h>
#include <geos/io/ByteOrderValues.h>
#include <geos/io/ParseException.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

//#define DEBUG_WKB_READER 1


using namespace geos::geom;

namespace geos {
namespace io { // geos.io

WKBReader::WKBReader(geom::GeometryFactory const& f)
    : factory(f)
    , inputDimension(2)
    , hasZ(false)
    , hasM(false)
    {}

WKBReader::WKBReader()
    : WKBReader(*(GeometryFactory::getDefaultInstance()))
    {}

std::ostream&
WKBReader::printHEX(std::istream& is, std::ostream& os)
{
    static const char hex[] = "0123456789ABCDEF";

    std::streampos pos = is.tellg(); // take note of input stream get pointer
    is.seekg(0, std::ios::beg); // rewind input stream

    char each = 0;
    while(is.read(&each, 1)) {
        const unsigned char c = static_cast<unsigned char>(each);
        int low = (c & 0x0F);
        int high = (c >> 4);
        os << hex[high] << hex[low];
    }

    is.clear(); // clear input stream eof flag
    is.seekg(pos); // reset input stream position

    return os;
}


namespace {

unsigned char
ASCIIHexToUChar(char val)
{
    switch(val) {
    case '0' :
        return 0;
    case '1' :
        return 1;
    case '2' :
        return 2;
    case '3' :
        return 3;
    case '4' :
        return 4;
    case '5' :
        return 5;
    case '6' :
        return 6;
    case '7' :
        return 7;
    case '8' :
        return 8;
    case '9' :
        return 9;
    case 'A' :
    case 'a' :
        return 10;
    case 'B' :
    case 'b' :
        return 11;
    case 'C' :
    case 'c' :
        return 12;
    case 'D' :
    case 'd' :
        return 13;
    case 'E' :
    case 'e' :
        return 14;
    case 'F' :
    case 'f' :
        return 15;
    default:
        throw ParseException("Invalid HEX char");
    }
}

}  // namespace

// Must be an even number of characters in the std::istream.
// Throws a ParseException if there are an odd number of characters.
std::unique_ptr<Geometry>
WKBReader::readHEX(std::istream& is)
{
    // setup input/output stream
    std::stringstream os(std::ios_base::binary | std::ios_base::in | std::ios_base::out);

    while(true) {
        const int input_high = is.get();
        if(input_high == std::char_traits<char>::eof()) {
            break;
        }

        const int input_low = is.get();
        if(input_low == std::char_traits<char>::eof()) {
            throw ParseException("Premature end of HEX string");
        }

        const char high = static_cast<char>(input_high);
        const char low = static_cast<char>(input_low);

        const unsigned char result_high = ASCIIHexToUChar(high);
        const unsigned char result_low = ASCIIHexToUChar(low);

        const unsigned char value =
            static_cast<unsigned char>((result_high << 4) + result_low);

#if DEBUG_HEX_READER
        std::size_t << "HEX " << high << low << " -> DEC " << (int)value << std::endl;
#endif
        // write the value to the output stream
        os << value;
    }

    // now call read to convert the geometry
    return this->read(os);
}

std::unique_ptr<Geometry>
WKBReader::read(std::istream& is)
{
    is.seekg(0, std::ios::end);
    auto size = is.tellg();
    is.seekg(0, std::ios::beg);

    std::vector<unsigned char> buf(static_cast<size_t>(size));
    is.read((char*) buf.data(), size);

    return read(buf.data(), buf.size());
}

std::unique_ptr<Geometry>
WKBReader::read(const unsigned char* buf, size_t size)
{
    dis = ByteOrderDataInStream(buf, size); // will default to machine endian
    return readGeometry();
}

std::unique_ptr<Geometry>
WKBReader::readGeometry()
{
    // determine byte order
    unsigned char byteOrder = dis.readByte();

#if DEBUG_WKB_READER
    std::size_t << "WKB byteOrder: " << (int)byteOrder << std::endl;
#endif

    // default is machine endian
    if(byteOrder == WKBConstants::wkbNDR) {
        dis.setOrder(ByteOrderValues::ENDIAN_LITTLE);
    }
    else if(byteOrder == WKBConstants::wkbXDR) {
        dis.setOrder(ByteOrderValues::ENDIAN_BIG);
    }

    uint32_t typeInt = dis.readUnsigned();
    /* Pick up both ISO and SFSQL geometry type */
    uint32_t geometryType = (typeInt & 0xffff) % 1000;
    /* ISO type range 1000 is Z, 2000 is M, 3000 is ZM */
    uint32_t isoTypeRange = (typeInt & 0xffff) / 1000;
    bool isoHasZ = (isoTypeRange == 1) || (isoTypeRange == 3);
    bool isoHasM = (isoTypeRange == 2) || (isoTypeRange == 3);
    /* SFSQL high bit flag for Z, next bit for M */
    int sfsqlHasZ = (typeInt & 0x80000000) != 0;
    int sfsqlHasM = (typeInt & 0x40000000) != 0;

#if DEBUG_WKB_READER
    std::size_t << "WKB geometryType: " << geometryType << std::endl;
#endif

    hasZ = sfsqlHasZ || isoHasZ;
    hasM = sfsqlHasM || isoHasM;
    if(hasZ && hasM) {
        inputDimension = 4;
    }
    else if (hasZ || hasM) {
        inputDimension = 3;
    }
    else {
        inputDimension = 2;
    }

#if DEBUG_WKB_READER
    std::size_t << "WKB hasZ: " << hasZ << std::endl;
#endif

#if DEBUG_WKB_READER
    std::size_t << "WKB dimensions: " << inputDimension << std::endl;
#endif

    bool hasSRID = ((typeInt & 0x20000000) != 0);

#if DEBUG_WKB_READER
    std::size_t << "WKB hasSRID: " << hasSRID << std::endl;
#endif

    int SRID = 0;
    if(hasSRID) {
        SRID = dis.readInt();    // read SRID
    }

    std::unique_ptr<Geometry> result;

    switch(geometryType) {
    case WKBConstants::wkbPoint :
        result = readPoint();
        break;
    case WKBConstants::wkbLineString :
        result = readLineString();
        break;
    case WKBConstants::wkbPolygon :
        result = readPolygon();
        break;
    case WKBConstants::wkbMultiPoint :
        result = readMultiPoint();
        break;
    case WKBConstants::wkbMultiLineString :
        result = readMultiLineString();
        break;
    case WKBConstants::wkbMultiPolygon :
        result = readMultiPolygon();
        break;
    case WKBConstants::wkbGeometryCollection :
        result = readGeometryCollection();
        break;
    default:
        std::stringstream err;
        err << "Unknown WKB type " << geometryType;
        throw  ParseException(err.str());
    }

    result->setSRID(SRID);
    return result;
}

std::unique_ptr<Point>
WKBReader::readPoint()
{
    readCoordinate();

    // POINT EMPTY
    if (std::isnan(curCoord.x) && std::isnan(curCoord.y)) {
        return std::unique_ptr<Point>(factory.createPoint(hasZ ? 3 : 2));
    }

    if (hasZ) {
        return std::unique_ptr<Point>(factory.createPoint(Coordinate(curCoord.x, curCoord.y, curCoord.z)));
    }
    else {
        return std::unique_ptr<Point>(factory.createPoint(Coordinate(curCoord.x, curCoord.y)));
    }
}

std::unique_ptr<LineString>
WKBReader::readLineString()
{
    uint32_t size = dis.readUnsigned();
#if DEBUG_WKB_READER
    std::size_t << "WKB npoints: " << size << std::endl;
#endif
    auto pts = readCoordinateSequence(size);
    return factory.createLineString(std::move(pts));
}

std::unique_ptr<LinearRing>
WKBReader::readLinearRing()
{
    uint32_t size = dis.readUnsigned();
#if DEBUG_WKB_READER
    std::size_t << "WKB npoints: " << size << std::endl;
#endif
    auto pts = readCoordinateSequence(size);
    return factory.createLinearRing(std::move(pts));
}

std::unique_ptr<Polygon>
WKBReader::readPolygon()
{
    uint32_t numRings = dis.readUnsigned();

#if DEBUG_WKB_READER
    std::size_t << "WKB numRings: " << numRings << std::endl;
#endif

    if(numRings == 0) {
        return factory.createPolygon(hasZ ? 3 : 2);
    }

    std::unique_ptr<LinearRing> shell;
    if(numRings > 0) {
        shell = readLinearRing();
    }

    if(numRings > 1) {
        std::vector<std::unique_ptr<LinearRing>> holes;
        for(uint32_t i = 0; i < numRings - 1; i++) {
            holes.push_back(std::move(readLinearRing()));
        }

        return factory.createPolygon(std::move(shell), std::move(holes));
    }
    return factory.createPolygon(std::move(shell));
}

std::unique_ptr<MultiPoint>
WKBReader::readMultiPoint()
{
    uint32_t numGeoms = dis.readUnsigned();
    std::vector<std::unique_ptr<Geometry>> geoms;

    for(uint32_t i = 0; i < numGeoms; i++) {
        auto geom = readGeometry();
        if(!dynamic_cast<Point*>(geom.get())) {
            std::stringstream err;
            err << BAD_GEOM_TYPE_MSG << " MultiPoint";
            throw ParseException(err.str());
        }
        geoms.push_back(std::move(geom));
    }

    return factory.createMultiPoint(std::move(geoms));
}

std::unique_ptr<MultiLineString>
WKBReader::readMultiLineString()
{
    uint32_t numGeoms = dis.readUnsigned();
    std::vector<std::unique_ptr<Geometry>> geoms;

    for(uint32_t i = 0; i < numGeoms; i++) {
        auto geom = readGeometry();
        if(!dynamic_cast<LineString*>(geom.get())) {
            std::stringstream err;
            err << BAD_GEOM_TYPE_MSG << " LineString";
            throw  ParseException(err.str());
        }
        geoms.push_back(std::move(geom));
    }

    return factory.createMultiLineString(std::move(geoms));
}

std::unique_ptr<MultiPolygon>
WKBReader::readMultiPolygon()
{
    uint32_t numGeoms = dis.readUnsigned();
    std::vector<std::unique_ptr<Geometry>> geoms;

    for(uint32_t i = 0; i < numGeoms; i++) {
        auto geom = readGeometry();
        if(!dynamic_cast<Polygon*>(geom.get())) {
            std::stringstream err;
            err << BAD_GEOM_TYPE_MSG << " Polygon";
            throw ParseException(err.str());
        }
        geoms.push_back(std::move(geom));
    }

    return factory.createMultiPolygon(std::move(geoms));
}

std::unique_ptr<GeometryCollection>
WKBReader::readGeometryCollection()
{
    uint32_t numGeoms = dis.readUnsigned();
    std::vector<std::unique_ptr<Geometry>> geoms;

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms.push_back(std::move(readGeometry()));
    }

    return factory.createGeometryCollection(std::move(geoms));
}

std::unique_ptr<CoordinateSequence>
WKBReader::readCoordinateSequence(uint32_t size)
{
    uint32_t targetDim = 2 + (hasZ ? 1 : 0);
    // auto seq = factory.getCoordinateSequenceFactory()->create(size, targetDim);
    if(targetDim > inputDimension) {
        targetDim = inputDimension;
    }
    curCoords.clear();
    for(uint32_t i = 0; i < size; i++) {
        readCoordinate();
        if (targetDim < 3) {
            curCoord.z = std::numeric_limits<double>::quiet_NaN();
        }
        curCoords.push_back(curCoord);
    }
    auto csFactory = factory.getCoordinateSequenceFactory();
    auto seq = csFactory->create(std::move(curCoords), targetDim);

    return seq;
}

void
WKBReader::readCoordinate()
{
    const PrecisionModel& pm = *factory.getPrecisionModel();
    // Read X
    curCoord.x = pm.makePrecise(dis.readDouble());
    // Read Y
    curCoord.y = pm.makePrecise(dis.readDouble());
    // Read Z if it's there
    if (hasZ) {
        curCoord.z = dis.readDouble();
    }
    if (hasM) {
        // Read and throw away any extra (M) dimensions
        dis.readDouble();
    }
#if DEBUG_WKB_READER
    std::size_t << "WKB coordinate: " << ordValues[0] << "," << ordValues[1] << std::endl;
#endif
}

} // namespace geos.io
} // namespace geos
