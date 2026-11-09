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
#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiCurve.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/MultiSurface.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util.h>

#include <array>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

//#define DEBUG_WKB_READER 1

#if DEBUG_WKB_READER
#include <iostream>
#endif

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

    // Convert and write in blocks
    char in[4096];
    char out[2 * sizeof(in)];
    bool first = true;
    while(is.read(in, sizeof(in)) || is.gcount() > 0) {
        const std::size_t n = static_cast<std::size_t>(is.gcount());
        for(std::size_t i = 0; i < n; ++i) {
            const unsigned char c = static_cast<unsigned char>(in[i]);
            out[2 * i] = hex[c >> 4];
            out[2 * i + 1] = hex[c & 0x0F];
        }

        std::size_t start = 0;
        if(first) {
            // Write the first digit with operator<< so that a width set
            // on os applies to it
            os << out[0];
            start = 1;
            first = false;
        }
        os.write(out + start, static_cast<std::streamsize>(2 * n - start));
    }

    is.clear(); // clear input stream eof flag
    is.seekg(pos); // reset input stream position

    return os;
}


namespace {

constexpr std::array<unsigned char, 256>
makeHexDigitValues()
{
    std::array<unsigned char, 256> values{};
    for(std::size_t i = 0; i < values.size(); ++i) {
        values[i] = 0xFF;
    }
    for(std::size_t i = 0; i < 10; ++i) {
        values['0' + i] = static_cast<unsigned char>(i);
    }
    for(std::size_t i = 0; i < 6; ++i) {
        values['A' + i] = static_cast<unsigned char>(10 + i);
        values['a' + i] = static_cast<unsigned char>(10 + i);
    }
    return values;
}

// Hex digit value by character, 0xFF for non-digits
constexpr std::array<unsigned char, 256> hexDigitValues = makeHexDigitValues();

unsigned char
decodeHexPair(int high, int low)
{
    const unsigned char h = hexDigitValues[static_cast<unsigned char>(high)];
    const unsigned char l = hexDigitValues[static_cast<unsigned char>(low)];
    if((h | l) > 0x0F) {
        throw ParseException("Invalid HEX char");
    }
    return static_cast<unsigned char>((h << 4) | l);
}

}  // namespace

// Must be an even number of characters in the std::istream.
// Throws a ParseException if there are an odd number of characters.
std::unique_ptr<Geometry>
WKBReader::readHEX(std::istream& is)
{
    std::vector<unsigned char> buf;

    // Read the stream buffer directly. Consumes the same characters and
    // sets the same state bits as calling is.get() per character.
    const std::istream::sentry ok(is, true);
    if(ok) {
        std::streambuf& sb = *is.rdbuf();
        const int eof = std::char_traits<char>::eof();

        while(true) {
            const int high = sb.sbumpc();
            if(high == eof) {
                is.setstate(std::ios_base::eofbit | std::ios_base::failbit);
                break;
            }

            const int low = sb.sbumpc();
            if(low == eof) {
                is.setstate(std::ios_base::eofbit | std::ios_base::failbit);
                throw ParseException("Premature end of HEX string");
            }

            buf.push_back(decodeHexPair(high, low));
        }
    }

    // now call read to convert the geometry
    return read(buf.data(), buf.size());
}

std::unique_ptr<Geometry>
WKBReader::readHEX(const std::string& hex)
{
    // Same checks, in the same order, as readHEX(std::istream&)
    std::vector<unsigned char> buf(hex.size() / 2);
    for(std::size_t i = 0; i < buf.size(); ++i) {
        buf[i] = decodeHexPair(hex[2 * i], hex[2 * i + 1]);
    }
    if(hex.size() % 2 != 0) {
        throw ParseException("Premature end of HEX string");
    }

    return read(buf.data(), buf.size());
}

void
WKBReader::minMemSize(geom::GeometryTypeId geomType, uint64_t size) const
{
    constexpr uint64_t minCoordSize = 2 * sizeof(double);
    constexpr uint64_t minPtSize = (1+4) + minCoordSize;
    constexpr uint64_t minLineSize = (1+4+4); // empty line
    constexpr uint64_t minRingSize = 4; // empty ring
    constexpr uint64_t minPolySize = (1+4+4); // empty polygon
    constexpr uint64_t minGeomSize = minLineSize;

    uint64_t perElement = 0;
    switch(geomType) {
        case GEOS_LINESTRING:
        case GEOS_LINEARRING:
        case GEOS_CIRCULARSTRING:
        case GEOS_COMPOUNDCURVE:
        case GEOS_POINT:
            perElement = minCoordSize;
            break;
        case GEOS_POLYGON:
        case GEOS_CURVEPOLYGON:
            perElement = minRingSize;
            break;
        case GEOS_MULTIPOINT:
            perElement = minPtSize;
            break;
        case GEOS_MULTILINESTRING:
        case GEOS_MULTICURVE:
            perElement = minLineSize;
            break;
        case GEOS_MULTIPOLYGON:
        case GEOS_MULTISURFACE:
            perElement = minPolySize;
            break;
        case GEOS_GEOMETRYCOLLECTION:
            perElement = minGeomSize;
            break;
    }

    if (perElement > 0 && size > dis.size() / perElement) {
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
    parseDepth_ = 0;
    dis = ByteOrderDataInStream(buf, size); // will default to machine endian
    return readGeometry();
}

static constexpr int MAX_PARSE_DEPTH = 100;

std::unique_ptr<Geometry>
WKBReader::readGeometry()
{
    if (parseDepth_ >= MAX_PARSE_DEPTH) {
        throw ParseException("Input geometry exceeds nesting depth limit");
    }
    ++parseDepth_;
    struct DepthGuard { int& d; ~DepthGuard() { --d; } } guard{parseDepth_};

    // determine byte order
    unsigned char byteOrder = dis.readByte();

#if DEBUG_WKB_READER
    std::cout << "WKB byteOrder: " << (int)byteOrder << std::endl;
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
    std::cout << "WKB geometryType: " << geometryType << std::endl;
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
    std::cout << "WKB hasZ: " << hasZ << std::endl;
#endif

#if DEBUG_WKB_READER
    std::cout << "WKB dimensions: " << inputDimension << std::endl;
#endif

    bool hasSRID = ((typeInt & 0x20000000) != 0);

#if DEBUG_WKB_READER
    std::cout << "WKB hasSRID: " << hasSRID << std::endl;
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
    case WKBConstants::wkbCircularString :
        result = readCircularString();
        break;
    case WKBConstants::wkbCompoundCurve :
        result = readCompoundCurve();
        break;
    case WKBConstants::wkbPolygon :
        result = readPolygon();
        break;
    case WKBConstants::wkbCurvePolygon :
        result = readCurvePolygon();
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
    case WKBConstants::wkbMultiCurve :
        result = readMultiCurve();
        break;
    case WKBConstants::wkbMultiSurface :
        result = readMultiSurface();
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
    std::cout << "WKB npoints: " << size << std::endl;
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
    std::cout << "WKB npoints: " << size << std::endl;
#endif
    auto pts = readCoordinateSequence(size);
    // Replace unclosed ring with closed
    if (fixStructure && !pts->isRing()) {
        pts->closeRing();
    }
    return factory.createLinearRing(std::move(pts));
}

std::unique_ptr<CircularString>
WKBReader::readCircularString()
{
    uint32_t size = dis.readUnsigned();
    minMemSize(GEOS_CIRCULARSTRING, size);
    auto pts = readCoordinateSequence(size);
    return factory.createCircularString(std::move(pts));
}

std::unique_ptr<CompoundCurve>
WKBReader::readCompoundCurve()
{
    auto numCurves = dis.readUnsigned();
    minMemSize(GEOS_COMPOUNDCURVE, numCurves);

    std::vector<std::unique_ptr<SimpleCurve>> curves(numCurves);

    for (std::uint32_t i = 0; i < numCurves; i++) {
        curves[i] = readChild<SimpleCurve>();
    }

    return factory.createCompoundCurve(std::move(curves));
}

std::unique_ptr<Polygon>
WKBReader::readPolygon()
{
    uint32_t numRings = dis.readUnsigned();
    minMemSize(GEOS_POLYGON, numRings);

#if DEBUG_WKB_READER
    std::cout << "WKB numRings: " << numRings << std::endl;
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

std::unique_ptr<CurvePolygon>
WKBReader::readCurvePolygon()
{
    uint32_t numRings = dis.readUnsigned();
    minMemSize(GEOS_POLYGON, numRings);

#if DEBUG_WKB_READER
    std::cout << "WKB numRings: " << numRings << std::endl;
#endif

    if (numRings == 0) {
        return factory.createCurvePolygon(hasZ, hasM);
    }

    auto shell = readChild<Curve>();

    if(numRings > 1) {
        std::vector<std::unique_ptr<Curve>> holes(numRings - 1);
        for(uint32_t i = 0; i < numRings - 1; i++) {
            holes[i] = readChild<Curve>();
        }

        return factory.createCurvePolygon(std::move(shell), std::move(holes));
    }

    return factory.createCurvePolygon(std::move(shell));
}

std::unique_ptr<MultiPoint>
WKBReader::readMultiPoint()
{
    uint32_t numGeoms = dis.readUnsigned();
    minMemSize(GEOS_MULTIPOINT, numGeoms);
    std::vector<std::unique_ptr<Geometry>> geoms(numGeoms);

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms[i] = readChild<Point>();
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
        geoms[i] = readChild<LineString>();
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
        geoms[i] = readChild<Polygon>();
    }

    return factory.createMultiPolygon(std::move(geoms));
}

std::unique_ptr<MultiCurve>
WKBReader::readMultiCurve()
{
    uint32_t numGeoms = dis.readUnsigned();
    minMemSize(GEOS_MULTICURVE, numGeoms);
    std::vector<std::unique_ptr<Curve>> geoms(numGeoms);

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms[i] = readChild<Curve>();
    }

    return factory.createMultiCurve(std::move(geoms));
}

std::unique_ptr<MultiSurface>
WKBReader::readMultiSurface()
{
    uint32_t numGeoms = dis.readUnsigned();
    minMemSize(GEOS_MULTISURFACE, numGeoms);
    std::vector<std::unique_ptr<Surface>> geoms(numGeoms);

    for(uint32_t i = 0; i < numGeoms; i++) {
        geoms[i] = readChild<Surface>();
    }

    return factory.createMultiSurface(std::move(geoms));
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
    std::cout << "WKB coordinate: " << ordValues[0] << "," << ordValues[1] << std::endl;
#endif
}

} // namespace geos.io
} // namespace geos
