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
#include "platform.h"
#include "geom.h"
#include "util.h"

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
	double getNVal();
	string getSVal();
private:
	string str;
	string stok;
	double ntok;
};

class WKTReader {
public:
	WKTReader();
	WKTReader(GeometryFactory *gf);
	~WKTReader();
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
	GeometryFactory *geometryFactory;
	PrecisionModel *precisionModel;
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

class WKTWriter {
public:
	WKTWriter();
	~WKTWriter();
	static string stringOfChar(char ch, int count);
	string write(const Geometry *geometry);
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
