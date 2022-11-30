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
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util.h>

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
    , fixStructure(false)
    {}

WKBReader::WKBReader()
    : WKBReader(*(GeometryFactory::getDefaultInstance()))
    {}

void
WKBReader::setFixStructure(bool doFixStructure)
{
    fixStructure = doFixStructure;
}

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

void
WKBReader::minMemSize(int geomType, uint64_t size)
{
    uint64_t minSize = 0;
    constexpr uint64_t minCoordSize = 2 * sizeof(double);
    constexpr uint64_t minPtSize = (1+4) + minCoordSize;
    constexpr uint64_t minLineSize = (1+4+4); // empty line
    constexpr uint64_t minRingSize = 4; // empty ring
    constexpr uint64_t minPolySize = (1+4+4); // empty polygon
    constexpr uint64_t minGeomSize = minLineSize;

    switch(geomType) {
        case GEOS_LINESTRING:
        case GEOS_LINEARRING:
            minSize = size * minCoordSize;
            break;
        case GEOS_POLYGON:
            minSize = size * minRingSize;
            break;
        case GEOS_MULTIPOINT:
            minSize = size * minPtSize;
            break;
        case GEOS_MULTILINESTRING:
            minSize = size * minLineSize;
            break;
        case GEOS_MULTIPOLYGON:
            minSize = size * minPolySize;
            break;
        case GEOS_GEOMETRYCOLLECTION:
            minSize = size * minGeomSize;
            break;
    }

    if (dis.size() < minSize) {
        throw ParseException("Input buffer is smaller than requested object size");
    }
}


std::unique_ptr<Geometry>
WKBReader::read(std::istream& is)
{
    is.seekg(0, std::ios::end);
    auto size = is.tellg();
    is.seekg(0, std::ios::beg);

    std::vector<unsigned char> buf(static_cast<size_t>(size));
    is.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(size));

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
    auto seq = readCoordinateSequence(1);

    // POINT EMPTY
    const CoordinateXY& coord = seq->getAt<CoordinateXY>(0);
    if (std::isnan(coord.x) && std::isnan(coord.y)) {
        seq->clear();
    }

    return factory.createPoint(std::move(seq));
}

std::unique_ptr<LineString>
WKBReader::readLineString()
{
    uint32_t size = dis.readUnsigned();
    minMemSize(GEOS_LINESTRING, size);
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
    minMemSize(GEOS_LINEARRING, size);
#if DEBUG_WKB_READER
    std::size_t << "WKB npoints: " << size << std::endl;
#endif
    auto pts = readCoordinateSequence(size);
    // Replace unclosed ring with closed
    if (fixStructure && !pts->isRing()) {
        pts->closeRing();
    }
    return factory.createLinearRing(std::move(pts));
}

std::unique_ptr<Polygon>
WKBReader::readPolygon()
{
    uint32_t numRings = dis.readUnsigned();
    minMemSize(GEOS_POLYGON, numRings);

#if DEBUG_WKB_READER
    std::size_t << "WKB numRings: " << numRings << std::endl;
#endif

    std::unique_ptr<LinearRing> shell;
    if (numRings == 0) {
        auto coords = detail::make_unique<CoordinateSequence>(0u, hasZ, hasM);
        shell = factory.createLinearRing(std::move(coords));
    } else {
        shell = readLinearRing();
    }

    if(numRings > 1) {
        std::vector<std::unique_ptr<LinearRing>> holes(numRings - 1);
        for(uint32_t i = 0; i < numRings - 1; i++) {
            holes[i] = readLinearRing();
        }

        return factory.createPolygon(std::move(shell), std::move(holes));
    }
    return factory.createPolygon(std::move(shell));
}

std::unique_ptr<MultiPoint>
WKBReader::readMultiPoint()
{
    uint32_t numGeoms = dis.readUnsigned();
    minMemSize(GEOS_MULTIPOINT, numGeoms);
    std::vector<std::unique_ptr<Geometry>> geoms(numGeoms);

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms[i] = readGeometry();
        if(!dynamic_cast<Point*>(geoms[i].get())) {
            std::stringstream err;
            err << BAD_GEOM_TYPE_MSG << " MultiPoint";
            throw ParseException(err.str());
        }
    }

    return factory.createMultiPoint(std::move(geoms));
}

std::unique_ptr<MultiLineString>
WKBReader::readMultiLineString()
{
    uint32_t numGeoms = dis.readUnsigned();
    minMemSize(GEOS_MULTILINESTRING, numGeoms);
    std::vector<std::unique_ptr<Geometry>> geoms(numGeoms);

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms[i] = readGeometry();
        if(!dynamic_cast<LineString*>(geoms[i].get())) {
            std::stringstream err;
            err << BAD_GEOM_TYPE_MSG << " LineString";
            throw  ParseException(err.str());
        }
    }

    return factory.createMultiLineString(std::move(geoms));
}

std::unique_ptr<MultiPolygon>
WKBReader::readMultiPolygon()
{
    uint32_t numGeoms = dis.readUnsigned();
    minMemSize(GEOS_MULTIPOLYGON, numGeoms);
    std::vector<std::unique_ptr<Geometry>> geoms(numGeoms);

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms[i] = readGeometry();
        if(!dynamic_cast<Polygon*>(geoms[i].get())) {
            std::stringstream err;
            err << BAD_GEOM_TYPE_MSG << " Polygon";
            throw ParseException(err.str());
        }
    }

    return factory.createMultiPolygon(std::move(geoms));
}

std::unique_ptr<GeometryCollection>
WKBReader::readGeometryCollection()
{
    uint32_t numGeoms = dis.readUnsigned();
    minMemSize(GEOS_GEOMETRYCOLLECTION, numGeoms);
    std::vector<std::unique_ptr<Geometry>> geoms(numGeoms);

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms[i] = readGeometry();
    }

    return factory.createGeometryCollection(std::move(geoms));
}

std::unique_ptr<CoordinateSequence>
WKBReader::readCoordinateSequence(uint32_t size)
{
    minMemSize(GEOS_LINESTRING, size);
    auto seq = detail::make_unique<CoordinateSequence>(size, hasZ, hasM, false);

    CoordinateXYZM coord(0, 0, DoubleNotANumber, DoubleNotANumber);
    for(uint32_t i = 0; i < size; i++) {
        readCoordinate();

        unsigned int j = 0;
        coord.x = ordValues[j++];
        coord.y = ordValues[j++];
        if (hasZ) {
            coord.z = ordValues[j++];
        }
        if (hasM) {
            coord.m = ordValues[j++];
        }

        seq->setAt(coord, i);
    }
    return seq;
}

void
WKBReader::readCoordinate()
{
    const PrecisionModel& pm = *factory.getPrecisionModel();

    for(std::size_t i = 0; i < inputDimension; ++i) {
        if (i < 2) {
            ordValues[i] = pm.makePrecise(dis.readDouble());
        } else {
            ordValues[i] = dis.readDouble();
        }
    }
#if DEBUG_WKB_READER
    std::size_t << "WKB coordinate: " << ordValues[0] << "," << ordValues[1] << std::endl;
#endif
}

} // namespace geos.io
} // namespace geos
