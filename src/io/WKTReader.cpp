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

#include <geos/io/WKTReader.h>
#include <geos/io/StringTokenizer.h>
#include <geos/io/ParseException.h>
#include <geos/io/CLocalizer.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/util.h>
#include <geos/util/string.h>

#include <sstream>
#include <string>
#include <cassert>


using namespace geos::geom;

namespace geos {
namespace io { // geos.io

std::unique_ptr<Geometry>
WKTReader::read(const std::string& wellKnownText) const
{
    CLocalizer clocale;
    StringTokenizer tokenizer(wellKnownText);
    OrdinateSet ordinateFlags = OrdinateSet::createXY();
    auto ret = readGeometryTaggedText(&tokenizer, ordinateFlags);

    if (tokenizer.peekNextToken() != StringTokenizer::TT_EOF) {
        tokenizer.nextToken();
        throw ParseException("Unexpected text after end of geometry");
    }

    return ret;
}

std::unique_ptr<CoordinateSequence>
WKTReader::getCoordinates(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    std::string nextToken = getNextEmptyOrOpener(tokenizer, ordinateFlags);
    if(nextToken == "EMPTY") {
        return detail::make_unique<CoordinateSequence>(0u, ordinateFlags.hasZ(), ordinateFlags.hasM());
    }

    CoordinateXYZM coord(0, 0, DoubleNotANumber, DoubleNotANumber);
    getPreciseCoordinate(tokenizer, ordinateFlags, coord);

    auto coordinates = detail::make_unique<CoordinateSequence>(0u, ordinateFlags.hasZ(), ordinateFlags.hasM());
    coordinates->add(coord);

    nextToken = getNextCloserOrComma(tokenizer);
    while(nextToken == ",") {
        getPreciseCoordinate(tokenizer, ordinateFlags, coord);
        coordinates->add(coord);
        nextToken = getNextCloserOrComma(tokenizer);
    }

    return coordinates;
}

void
WKTReader::getPreciseCoordinate(StringTokenizer* tokenizer,
                                OrdinateSet& ordinateFlags,
                                CoordinateXYZM& coord) const {
    coord.x = getNextNumber(tokenizer);
    coord.y = getNextNumber(tokenizer);

    // Check for undeclared Z dimension
    if (ordinateFlags.changesAllowed() && isNumberNext(tokenizer)) {
        ordinateFlags.setZ(true);
    }

    if (ordinateFlags.hasZ()) {
        coord.z = getNextNumber(tokenizer);
    }

    // Check for undeclared M dimension
    if (ordinateFlags.changesAllowed() && ordinateFlags.hasZ() && isNumberNext(tokenizer)) {
        ordinateFlags.setM(true);
    }

    if (ordinateFlags.hasM()) {
        coord.m = getNextNumber(tokenizer);
    }

    ordinateFlags.setChangesAllowed(false); // First coordinate read; future coordinates must be consistent

    precisionModel->makePrecise(coord);
}

bool
WKTReader::isNumberNext(StringTokenizer* tokenizer)
{
    return tokenizer->peekNextToken() == StringTokenizer::TT_NUMBER;
}

bool
WKTReader::isOpenerNext(StringTokenizer* tokenizer)
{
    return tokenizer->peekNextToken() == '(';
}

double
WKTReader::getNextNumber(StringTokenizer* tokenizer)
{
    int type = tokenizer->nextToken();
    switch(type) {
    case StringTokenizer::TT_EOF:
        throw  ParseException("Expected number but encountered end of stream");
    case StringTokenizer::TT_EOL:
        throw  ParseException("Expected number but encountered end of line");
    case StringTokenizer::TT_NUMBER:
        return tokenizer->getNVal();
    case StringTokenizer::TT_WORD:
        throw  ParseException("Expected number but encountered word", tokenizer->getSVal());
    case '(':
        throw  ParseException("Expected number but encountered '('");
    case ')':
        throw  ParseException("Expected number but encountered ')'");
    case ',':
        throw  ParseException("Expected number but encountered ','");
    }
    assert(0); // Encountered unexpected StreamTokenizer type
    return 0;
}

bool
WKTReader::isTypeName(const std::string & type, const std::string & typeName) {
    return util::startsWith(type, typeName);
}

void
WKTReader::readOrdinateFlags(const std::string & s, OrdinateSet& ordinateFlags) {
    if (util::endsWith(s, "ZM")) {
        ordinateFlags.setM(true);
        ordinateFlags.setZ(true);
        ordinateFlags.setChangesAllowed(false);
    } else if (util::endsWith(s, 'M')) {
        ordinateFlags.setM(true);
        ordinateFlags.setChangesAllowed(false);
    } else if (util::endsWith(s, 'Z')) {
        ordinateFlags.setZ(true);
        ordinateFlags.setChangesAllowed(false);
    }
}

std::string
WKTReader::getNextEmptyOrOpener(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags)
{
    std::string nextWord = getNextWord(tokenizer);

    bool flagsModified = false;

    // Skip the Z, M or ZM of an SF1.2 3/4 dim coordinate.
    if (nextWord == "ZM") {
        ordinateFlags.setZ(true);
        ordinateFlags.setM(true);
        flagsModified = true;
        nextWord = getNextWord(tokenizer);
    } else {
        if (nextWord == "Z") {
            ordinateFlags.setZ(true);
            flagsModified = true;
            nextWord = getNextWord(tokenizer);
        }

        if (nextWord == "M") {
            ordinateFlags.setM(true);
            flagsModified = true;
            nextWord = getNextWord(tokenizer);
        }
    }

    if (flagsModified) {
        ordinateFlags.setChangesAllowed(false);
    }

    if(nextWord == "EMPTY" || nextWord == "(") {
        return nextWord;
    }
    throw ParseException("Expected 'Z', 'M', 'ZM', 'EMPTY' or '(' but encountered ", nextWord);
}

std::string
WKTReader::getNextCloserOrComma(StringTokenizer* tokenizer)
{
    std::string nextWord = getNextWord(tokenizer);
    if(nextWord == "," || nextWord == ")") {
        return nextWord;
    }
    throw  ParseException("Expected ')' or ',' but encountered", nextWord);
}

std::string
WKTReader::getNextCloser(StringTokenizer* tokenizer)
{
    std::string nextWord = getNextWord(tokenizer);
    if(nextWord == ")") {
        return nextWord;
    }
    throw  ParseException("Expected ')' but encountered", nextWord);
}

std::string
WKTReader::getNextWord(StringTokenizer* tokenizer)
{
    int type = tokenizer->nextToken();
    switch(type) {
    case StringTokenizer::TT_EOF:
        throw  ParseException("Expected word but encountered end of stream");
    case StringTokenizer::TT_EOL:
        throw  ParseException("Expected word but encountered end of line");
    case StringTokenizer::TT_NUMBER:
        throw  ParseException("Expected word but encountered number", tokenizer->getNVal());
    case StringTokenizer::TT_WORD: {
        std::string word = tokenizer->getSVal();
        for (char& c : word) {
            // Avoid UB if c is not representable as unsigned char
            // https://en.cppreference.com/w/cpp/string/byte/toupper
            c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
        }
        return word;
    }
    case '(':
        return "(";
    case ')':
        return ")";
    case ',':
        return ",";
    }
    assert(0);
    //throw  ParseException("Encountered unexpected StreamTokenizer type");
    return "";
}

std::unique_ptr<Geometry>
WKTReader::readGeometryTaggedText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    std::string type = getNextWord(tokenizer);

    std::unique_ptr<Geometry> geom;
    OrdinateSet origFlags = ordinateFlags;
    OrdinateSet newFlags = OrdinateSet::createXY();
    readOrdinateFlags(type, newFlags);

    if(isTypeName(type, "POINT")) {
        geom = readPointText(tokenizer, newFlags);
    }
    else if(isTypeName(type, "LINESTRING")) {
        geom = readLineStringText(tokenizer, newFlags);
    }
    else if(isTypeName(type, "LINEARRING")) {
        geom = readLinearRingText(tokenizer, newFlags);
    }
    else if(isTypeName(type, "POLYGON")) {
        geom = readPolygonText(tokenizer, newFlags);
    }
    else if(isTypeName(type,  "MULTIPOINT")) {
        geom = readMultiPointText(tokenizer, newFlags);
    }
    else if(isTypeName(type, "MULTILINESTRING")) {
        geom = readMultiLineStringText(tokenizer, newFlags);
    }
    else if(isTypeName(type, "MULTIPOLYGON")) {
        geom = readMultiPolygonText(tokenizer, newFlags);
    }
    else if(isTypeName(type, "GEOMETRYCOLLECTION")) {
        geom = readGeometryCollectionText(tokenizer, newFlags);
    } else {
        throw ParseException("Unknown type", type);
    }

    if (!origFlags.changesAllowed() && newFlags != origFlags) {
        throw ParseException("Cannot mix dimensionality in a geometry.");
    }

    return geom;

}

std::unique_ptr<Point>
WKTReader::readPointText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    auto coords = getCoordinates(tokenizer, ordinateFlags);
    return geometryFactory->createPoint(std::move(coords));
}

std::unique_ptr<LineString>
WKTReader::readLineStringText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    auto&& coords = getCoordinates(tokenizer, ordinateFlags);
    return geometryFactory->createLineString(std::move(coords));
}

std::unique_ptr<LinearRing>
WKTReader::readLinearRingText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    auto&& coords = getCoordinates(tokenizer, ordinateFlags);
    if (fixStructure && !coords->isRing()) {
        coords->closeRing();
    }
    return geometryFactory->createLinearRing(std::move(coords));
}

std::unique_ptr<MultiPoint>
WKTReader::readMultiPointText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    std::string nextToken = getNextEmptyOrOpener(tokenizer, ordinateFlags);
    if(nextToken == "EMPTY") {
        return geometryFactory->createMultiPoint();
    }

    int tok = tokenizer->peekNextToken();

    if(tok == StringTokenizer::TT_NUMBER) {
        // Try to parse "MULTIPOINT (0 0, 1 1)"
        auto coords = detail::make_unique<CoordinateSequence>(0u, ordinateFlags.hasZ(), ordinateFlags.hasM());

        CoordinateXYZM coord(0, 0, DoubleNotANumber, DoubleNotANumber);
        do {
            getPreciseCoordinate(tokenizer, ordinateFlags, coord);
            coords->add(coord);
            nextToken = getNextCloserOrComma(tokenizer);
        }
        while(nextToken == ",");

        return std::unique_ptr<MultiPoint>(geometryFactory->createMultiPoint(*coords));
    }

    else if(tok == '(' ||        // Try to parse "MULTIPOINT ((0 0), (1 1))"
            tok == StringTokenizer::TT_WORD)  // "MULTIPOINT (EMPTY, (1 1))"
    {
        std::vector<std::unique_ptr<Point>> points;

        do {
            points.push_back(readPointText(tokenizer, ordinateFlags));
            nextToken = getNextCloserOrComma(tokenizer);
        } while(nextToken == ",");

        return geometryFactory->createMultiPoint(std::move(points));
    }

    else {
        std::stringstream err;
        err << "Unexpected token: ";
        switch(tok) {
        case StringTokenizer::TT_WORD:
            err << "WORD " << tokenizer->getSVal();
            break;
        case StringTokenizer::TT_NUMBER:
            err << "NUMBER " << tokenizer->getNVal();
            break;
        case StringTokenizer::TT_EOF:
        case StringTokenizer::TT_EOL:
            err << "EOF or EOL";
            break;
        case '(':
            err << "(";
            break;
        case ')':
            err << ")";
            break;
        case ',':
            err << ",";
            break;
        default:
            err << "??";
            break;
        }
        err << std::endl;
        throw ParseException(err.str());
    }
}

std::unique_ptr<Polygon>
WKTReader::readPolygonText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    std::string nextToken = getNextEmptyOrOpener(tokenizer, ordinateFlags);
    if(nextToken == "EMPTY") {
        auto coords = detail::make_unique<CoordinateSequence>(0u, ordinateFlags.hasZ(), ordinateFlags.hasM());
        auto ring = geometryFactory->createLinearRing(std::move(coords));
        return geometryFactory->createPolygon(std::move(ring));
    }

    std::vector<std::unique_ptr<LinearRing>> holes;
    auto shell = readLinearRingText(tokenizer, ordinateFlags);
    nextToken = getNextCloserOrComma(tokenizer);
    while(nextToken == ",") {
        holes.push_back(readLinearRingText(tokenizer, ordinateFlags));
        nextToken = getNextCloserOrComma(tokenizer);
    }

    return geometryFactory->createPolygon(std::move(shell), std::move(holes));
}

std::unique_ptr<MultiLineString>
WKTReader::readMultiLineStringText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    std::string nextToken = getNextEmptyOrOpener(tokenizer, ordinateFlags);
    if(nextToken == "EMPTY") {
        return geometryFactory->createMultiLineString();
    }

    std::vector<std::unique_ptr<LineString>> lineStrings;
    do {
        lineStrings.push_back(readLineStringText(tokenizer, ordinateFlags));
        nextToken = getNextCloserOrComma(tokenizer);
    } while (nextToken == ",");

    return geometryFactory->createMultiLineString(std::move(lineStrings));
}

std::unique_ptr<MultiPolygon>
WKTReader::readMultiPolygonText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    std::string nextToken = getNextEmptyOrOpener(tokenizer, ordinateFlags);
    if(nextToken == "EMPTY") {
        return geometryFactory->createMultiPolygon();
    }

    std::vector<std::unique_ptr<Polygon>> polygons;
    do {
        polygons.push_back(readPolygonText(tokenizer, ordinateFlags));
        nextToken = getNextCloserOrComma(tokenizer);
    } while(nextToken == ",");

    return geometryFactory->createMultiPolygon(std::move(polygons));
}

std::unique_ptr<GeometryCollection>
WKTReader::readGeometryCollectionText(StringTokenizer* tokenizer, OrdinateSet& ordinateFlags) const
{
    std::string nextToken = getNextEmptyOrOpener(tokenizer, ordinateFlags);
    if(nextToken == "EMPTY") {
        return geometryFactory->createGeometryCollection();
    }

    std::vector<std::unique_ptr<Geometry>> geoms;
    do {
        geoms.push_back(readGeometryTaggedText(tokenizer, ordinateFlags));
        nextToken = getNextCloserOrComma(tokenizer);
    } while(nextToken == ",");

    return geometryFactory->createGeometryCollection(std::move(geoms));
}

} // namespace geos.io
} // namespace geos
