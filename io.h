#include <iostream>
#include <string>
//#include <vector>
//#include <algorithm>
//#include "math.h"
#include "platform.h"
#include "geom.h"

using namespace std;


class ParseException {
public:
	ParseException();
	ParseException(string msg);
	ParseException(string msg, string var);
	ParseException(string msg, double num);
	~ParseException();
	string toString();
private:
	string txt;
};

class StringTokenizer {
public:
	static const int TT_EOF=0;
	static const int TT_EOL=1;
	static const int TT_NUMBER=2;
	static const int TT_WORD=3;
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
	WKTReader(GeometryFactory gf);
	~WKTReader();
	Geometry read(string wellKnownText);
//	Geometry read(Reader reader);	//Not implemented yet
protected:
	CoordinateList getCoordinates(StringTokenizer *tokenizer);
	double getNextNumber(StringTokenizer *tokenizer);
	string getNextEmptyOrOpener(StringTokenizer *tokenizer);
	string getNextCloserOrComma(StringTokenizer *tokenizer);
	string getNextCloser(StringTokenizer *tokenizer);
	string getNextWord(StringTokenizer *tokenizer);
	Geometry readGeometryTaggedText(StringTokenizer *tokenizer);
	Point readPointText(StringTokenizer *tokenizer);
	LineString readLineStringText(StringTokenizer *tokenizer);
	LinearRing readLinearRingText(StringTokenizer *tokenizer);
	MultiPoint readMultiPointText(StringTokenizer *tokenizer);
//  protected Point[] toPoints(Coordinate[] coordinates); //Not needed
	Polygon readPolygonText(StringTokenizer *tokenizer);
	MultiLineString readMultiLineStringText(StringTokenizer *tokenizer);
	MultiPolygon readMultiPolygonText(StringTokenizer *tokenizer);
	GeometryCollection readGeometryCollectionText(StringTokenizer *tokenizer);
private:
	GeometryFactory geometryFactory;
	PrecisionModel precisionModel;
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
	string write(Geometry *geometry);
	void write(Geometry *geometry, Writer *writer);
	string writeFormatted(Geometry *geometry);
	void writeFormatted(Geometry *geometry, Writer *writer);
protected:
	string formatter;
	void appendGeometryTaggedText(Geometry *geometry, int level, Writer *writer);
	void appendPointTaggedText(Coordinate coordinate, int level, Writer *writer,PrecisionModel precisionModel);
	void appendLineStringTaggedText(LineString *lineString, int level, Writer *writer);
	void appendPolygonTaggedText(Polygon *polygon, int level, Writer *writer);
	void appendMultiPointTaggedText(MultiPoint *multipoint, int level, Writer *writer);
	void appendMultiLineStringTaggedText(MultiLineString *multiLineString, int level,Writer *writer);
	void appendMultiPolygonTaggedText(MultiPolygon *multiPolygon, int level, Writer *writer);
	void appendGeometryCollectionTaggedText(GeometryCollection *geometryCollection, int level,Writer *writer);
	void appendPointText(Coordinate coordinate, int level, Writer *writer,PrecisionModel precisionModel);
	void appendCoordinate(Coordinate coordinate, Writer *writer, PrecisionModel precisionModel);
	string writeNumber(double d);
	void appendLineStringText(LineString *lineString, int level, bool doIndent, Writer *writer);
	void appendPolygonText(Polygon *polygon, int level, bool indentFirst, Writer *writer);
	void appendMultiPointText(MultiPoint *multiPoint, int level, Writer *writer);
	void appendMultiLineStringText(MultiLineString *multiLineString, int level, bool indentFirst,Writer *writer);
	void appendMultiPolygonText(MultiPolygon *multiPolygon, int level, Writer *writer);
	void appendGeometryCollectionText(GeometryCollection *geometryCollection, int level,Writer *writer);
private:
	static const int INDENT = 2;
	static string createFormatter(PrecisionModel precisionModel);
	bool isFormatted;
	int level;
	void writeFormatted(Geometry *geometry, bool isFormatted, Writer *writer);
	void indent(int level, Writer *writer);
};
