/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.2  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.15  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.14  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#ifndef GEOS_IO_H
#define GEOS_IO_H

#include <memory>
#include <iostream>
#include <string>
//#include <vector>
//#include <algorithm>
//#include "math.h"
#include <geos/platform.h>
#include <geos/geom.h>
#include <geos/util.h>

using namespace std;

namespace geos {

class ParseException: public GEOSException {
public:
	ParseException();
	ParseException(string msg);
	ParseException(string msg, string var);
	ParseException(string msg, double num);
	~ParseException();
};

class StringTokenizer {
public:
	enum {
		TT_EOF,
		TT_EOL,
		TT_NUMBER,
		TT_WORD
	};
	StringTokenizer();
	StringTokenizer(string txt);
	~StringTokenizer();
	int nextToken();
	int peekNextToken();
	double getNVal();
	string getSVal();
private:
	string str;
	string stok;
	double ntok;
};

/**
 * \class WKTReader io.h geos/io.h
 * \brief WKT parser class; see also WKTWriter.
 */
class WKTReader {
public:
	//WKTReader();

	/**
	 * \brief Inizialize parser with given GeometryFactory.
	 *
	 * Note that all Geometry objects created by the
	 * parser will contain a pointer to the given factory
	 * so be sure you'll keep the factory alive for the
	 * whole WKTReader and created Geometry life.
	 */
	WKTReader(const GeometryFactory *gf);

	~WKTReader();

	/// Parse a WKT string returning a Geometry
	Geometry* read(string wellKnownText);

//	Geometry* read(Reader reader);	//Not implemented yet

protected:
	CoordinateList* getCoordinates(StringTokenizer *tokenizer);
	double getNextNumber(StringTokenizer *tokenizer);
	string getNextEmptyOrOpener(StringTokenizer *tokenizer);
	string getNextCloserOrComma(StringTokenizer *tokenizer);
	string getNextCloser(StringTokenizer *tokenizer);
	string getNextWord(StringTokenizer *tokenizer);
	Geometry* readGeometryTaggedText(StringTokenizer *tokenizer);
	Point* readPointText(StringTokenizer *tokenizer);
	LineString* readLineStringText(StringTokenizer *tokenizer);
	LinearRing* readLinearRingText(StringTokenizer *tokenizer);
	MultiPoint* readMultiPointText(StringTokenizer *tokenizer);
	Polygon* readPolygonText(StringTokenizer *tokenizer);
	MultiLineString* readMultiLineStringText(StringTokenizer *tokenizer);
	MultiPolygon* readMultiPolygonText(StringTokenizer *tokenizer);
	GeometryCollection* readGeometryCollectionText(StringTokenizer *tokenizer);
private:
	const GeometryFactory *geometryFactory;
	const PrecisionModel *precisionModel;
	Coordinate* getPreciseCoordinate(StringTokenizer *tokenizer);
	bool isNumberNext(StringTokenizer *tokenizer);
};

class Writer {
public:
	Writer();
	~Writer();
	void write(string txt);
	string toString();
private:
	string str;
};

/**
 * \class WKTWriter io.h geos/io.h
 *
 * \brief Outputs the textual representation of a Geometry.
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
class WKTWriter {
public:
	WKTWriter();
	~WKTWriter();

	//string(count, ch) can be used for this
	//static string stringOfChar(char ch, int count);

	/// Returns WKT string for the given Geometry
	string write(const Geometry *geometry);

	// Send Geometry's WKT to the given Writer
	void write(const Geometry *geometry, Writer *writer);

	string writeFormatted(const Geometry *geometry);

	void writeFormatted(const Geometry *geometry, Writer *writer);

protected:
	string formatter;
	void appendGeometryTaggedText(const Geometry *geometry, int level, Writer *writer);
	void appendPointTaggedText(const Coordinate* coordinate, int level, Writer *writer, const PrecisionModel* precisionModel);
	void appendLineStringTaggedText(const LineString *lineString, int level, Writer *writer);
	void appendLinearRingTaggedText(const LinearRing *lineString, int level, Writer *writer);
	void appendPolygonTaggedText(const Polygon *polygon, int level, Writer *writer);
	void appendMultiPointTaggedText(const MultiPoint *multipoint, int level, Writer *writer);
	void appendMultiLineStringTaggedText(const MultiLineString *multiLineString, int level,Writer *writer);
	void appendMultiPolygonTaggedText(const MultiPolygon *multiPolygon, int level, Writer *writer);
	void appendGeometryCollectionTaggedText(const GeometryCollection *geometryCollection, int level,Writer *writer);
	void appendPointText(const Coordinate* coordinate, int level, Writer *writer, const PrecisionModel* precisionModel);
	void appendCoordinate(const Coordinate* coordinate, Writer *writer, const PrecisionModel* precisionModel);
	string writeNumber(double d);
	void appendLineStringText(const LineString *lineString, int level, bool doIndent, Writer *writer);
	void appendPolygonText(const Polygon *polygon, int level, bool indentFirst, Writer *writer);
	void appendMultiPointText(const MultiPoint *multiPoint, int level, Writer *writer);
	void appendMultiLineStringText(const MultiLineString *multiLineString, int level, bool indentFirst,Writer *writer);
	void appendMultiPolygonText(const MultiPolygon *multiPolygon, int level, Writer *writer);
	void appendGeometryCollectionText(const GeometryCollection *geometryCollection, int level,Writer *writer);
private:
	enum {
		INDENT = 2
	};
//	static const int INDENT = 2;
	static string createFormatter(const PrecisionModel* precisionModel);
	bool isFormatted;
	int level;
	void writeFormatted(const Geometry *geometry, bool isFormatted, Writer *writer);
	void indent(int level, Writer *writer);
};
}
#endif
