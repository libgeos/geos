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
	int currPos;
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