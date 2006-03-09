/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <string>
#include <cassert>

#include <geos/io.h>
#include <geos/util.h>

#include <geos/geom.h>

using namespace std;

namespace geos {
namespace io { // geos.io

//WKTReader::WKTReader(){
//	geometryFactory=new GeometryFactory();
//	precisionModel=geometryFactory->getPrecisionModel();
//}

WKTReader::WKTReader(const GeometryFactory *gf){
	geometryFactory=gf;
	precisionModel=gf->getPrecisionModel();
}

//WKTReader::~WKTReader(){ }

Geometry *
WKTReader::read(const string &wellKnownText)
{
	auto_ptr<StringTokenizer> tokenizer(new StringTokenizer(wellKnownText));
	StringTokenizer *st=tokenizer.release();
	Geometry *g=NULL;
	try {
		g=readGeometryTaggedText(st);
	}
	catch (...) {
		delete st;
		throw;
	}
	delete st;
	return g;
}

CoordinateSequence* WKTReader::getCoordinates(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return new CoordinateArraySequence(); 
	}
	CoordinateSequence *coordinates = new CoordinateArraySequence();
	Coordinate *coord = getPreciseCoordinate(tokenizer);
	coordinates->add(*coord);
	delete coord; coord=NULL;
	try {
		nextToken=getNextCloserOrComma(tokenizer);
		while (nextToken==",") {
			coord = getPreciseCoordinate(tokenizer);
			coordinates->add(*coord);
			delete coord; coord=NULL;
			nextToken=getNextCloserOrComma(tokenizer);
		}
	} catch (...) {
		delete coord;
		delete coordinates;
		throw;
	}
	return coordinates;
}

Coordinate* WKTReader::getPreciseCoordinate(StringTokenizer *tokenizer) {
	Coordinate *coord=new Coordinate();
	try {
		coord->x=getNextNumber(tokenizer);
		coord->y=getNextNumber(tokenizer);
		if (isNumberNext(tokenizer)) {
			coord->z=getNextNumber(tokenizer);
		}
		precisionModel->makePrecise(coord);
	} catch (...) {
		delete coord;
		throw;
	}
	return coord;
}

bool WKTReader::isNumberNext(StringTokenizer *tokenizer) {
	return tokenizer->peekNextToken()==StringTokenizer::TT_NUMBER;
}

double WKTReader::getNextNumber(StringTokenizer *tokenizer) {
	int type=tokenizer->nextToken();
	switch(type){
		case StringTokenizer::TT_EOF:
			throw  ParseException("Expected number but encountered end of stream");
		case StringTokenizer::TT_EOL:
			throw  ParseException("Expected number but encountered end of line");
		case StringTokenizer::TT_NUMBER:
			return tokenizer->getNVal();
		case StringTokenizer::TT_WORD:
			throw  ParseException("Expected number but encountered word",tokenizer->getSVal());
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

string WKTReader::getNextEmptyOrOpener(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord=="EMPTY" || nextWord=="(") {
		return nextWord;
	}
	throw  ParseException("Expected 'EMPTY' or '(' but encountered ",nextWord);
}

string WKTReader::getNextCloserOrComma(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord=="," || nextWord==")") {
		return nextWord;
	}
	throw  ParseException("Expected ')' or ',' but encountered",nextWord);
}

string WKTReader::getNextCloser(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord==")") {
		return nextWord;
	}
	throw  ParseException("Expected ')' but encountered",nextWord);
}

string WKTReader::getNextWord(StringTokenizer *tokenizer) {
	int type=tokenizer->nextToken();
	switch(type){
		case StringTokenizer::TT_EOF:
			throw  ParseException("Expected word but encountered end of stream");
		case StringTokenizer::TT_EOL:
			throw  ParseException("Expected word but encountered end of line");
		case StringTokenizer::TT_NUMBER:
			throw  ParseException("Expected word but encountered number", tokenizer->getNVal());
		case StringTokenizer::TT_WORD:
			return tokenizer->getSVal();
		case '(':
			return "(";
		case ')':
			return ")";
		case ',':
			return ",";
	}
	assert(0); // Encountered unexpected StreamTokenizer type
	return "";
}

Geometry* WKTReader::readGeometryTaggedText(StringTokenizer *tokenizer) {
	string type = getNextWord(tokenizer);
	if (type=="POINT") {
		return readPointText(tokenizer);
	} else if (type=="LINESTRING") {
		return readLineStringText(tokenizer);
	} else if (type=="LINEARRING") {
		return readLinearRingText(tokenizer);
	} else if (type=="POLYGON") {
		return readPolygonText(tokenizer);
	} else if (type=="MULTIPOINT") {
		return readMultiPointText(tokenizer);
	} else if (type=="MULTILINESTRING") {
		return readMultiLineStringText(tokenizer);
	} else if (type=="MULTIPOLYGON") {
		return readMultiPolygonText(tokenizer);
	} else if (type=="GEOMETRYCOLLECTION") {
		return readGeometryCollectionText(tokenizer);
	}
	throw  ParseException("Unknown type",type);
}

Point* WKTReader::readPointText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createPoint(Coordinate::getNull());
	}
	Coordinate *coord = getPreciseCoordinate(tokenizer);
	Point *pt=geometryFactory->createPoint(*coord);
	delete coord;
	try {
		getNextCloser(tokenizer);
	} catch (...) {
		delete pt;
		throw;
	}
	return pt;
}

LineString* WKTReader::readLineStringText(StringTokenizer *tokenizer) {
	CoordinateSequence *coords = getCoordinates(tokenizer);
	LineString *ret = geometryFactory->createLineString(coords);
	return ret;
}

LinearRing* WKTReader::readLinearRingText(StringTokenizer *tokenizer) {
	CoordinateSequence *coords = getCoordinates(tokenizer);
	LinearRing *ret;
	ret = geometryFactory->createLinearRing(coords);
	return ret;
}

MultiPoint* WKTReader::readMultiPointText(StringTokenizer *tokenizer) {
	CoordinateSequence *coords = getCoordinates(tokenizer);
	MultiPoint *ret = geometryFactory->createMultiPoint(*coords);
	delete coords;
	return ret;
}

Polygon*
WKTReader::readPolygonText(StringTokenizer *tokenizer)
{
	Polygon *poly=NULL;
	LinearRing *shell=NULL;
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createPolygon(NULL,NULL);
	}

	vector<Geometry *> *holes=new vector<Geometry *>();
	try {
		shell=readLinearRingText(tokenizer);
		nextToken=getNextCloserOrComma(tokenizer);
		while(nextToken==",") {
			LinearRing *hole=readLinearRingText(tokenizer);
			holes->push_back(hole);
			nextToken=getNextCloserOrComma(tokenizer);
		}
		poly = geometryFactory->createPolygon(shell,holes);
	} catch (...) {
		for (unsigned int i=0; i<holes->size(); i++)
			delete (*holes)[i];
		delete holes;
		delete shell;
		throw;
	}
	return poly;
}

MultiLineString* WKTReader::readMultiLineStringText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createMultiLineString(NULL);
	}
	vector<Geometry *> *lineStrings=new vector<Geometry *>();
	LineString *lineString=readLineStringText(tokenizer);
	lineStrings->push_back(lineString);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		LineString *lineString=readLineStringText(tokenizer);
		lineStrings->push_back(lineString);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	MultiLineString *ret = geometryFactory->createMultiLineString(lineStrings);
	//for (int i=0; i<lineStrings->size(); i++) delete (*lineStrings)[i];
	//delete lineStrings;
	return ret;
}

MultiPolygon* WKTReader::readMultiPolygonText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createMultiPolygon(NULL);
	}
	vector<Geometry *> *polygons=new vector<Geometry *>();
	Polygon *polygon=readPolygonText(tokenizer);
	polygons->push_back(polygon);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		Polygon *polygon=readPolygonText(tokenizer);
		polygons->push_back(polygon);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	MultiPolygon *ret = geometryFactory->createMultiPolygon(polygons);
	//for (int i=0; i<polygons->size(); i++) delete (*polygons)[i];
	//delete polygons;
	return ret;
}

GeometryCollection* WKTReader::readGeometryCollectionText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createGeometryCollection(NULL);
	}
	vector<Geometry *> *geoms=new vector<Geometry *>();
	Geometry *geom;
	geom=readGeometryTaggedText(tokenizer);
	geoms->push_back(geom);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		geom=readGeometryTaggedText(tokenizer);
		geoms->push_back(geom);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	GeometryCollection *ret = geometryFactory->createGeometryCollection(geoms);
	//for (int i=0; i<geoms->size(); i++) delete (*geoms)[i];
	//delete geoms;
	return ret;
}

} // namespace geos.io
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.37  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.36  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.35  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.34  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.33  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.32  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.31  2005/04/14 11:49:02  strk
 * Applied slightly modified patch by Cheng Shan to speedup WKT parsing.
 *
 * Revision 1.30  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.29  2004/10/21 17:13:59  strk
 * Fixed bug introduced by previous patch.
 *
 * Revision 1.28  2004/10/21 07:03:31  strk
 * Removed leak in ::readPolygonText reported by Carlos A. Rueda
 *
 * Revision 1.27  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.26  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.25  2004/07/05 10:50:21  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.24  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.23  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.22  2004/06/15 20:30:00  strk
 * fixed a typo
 *
 * Revision 1.21  2004/06/15 20:16:19  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.20  2004/05/14 12:10:54  strk
 * avoided leaks on malformed LinearRing
 *
 * Revision 1.19  2004/05/07 13:23:51  strk
 * Memory leaks fixed.
 *
 * Revision 1.18  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.16  2003/10/15 08:52:55  strk
 * Memory leaks fixed.
 *
 **********************************************************************/
