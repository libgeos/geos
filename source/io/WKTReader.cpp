#include "io.h"
#include "util.h"

WKTReader::WKTReader(){
	geometryFactory=new GeometryFactory();
	precisionModel=new PrecisionModel();
}
WKTReader::WKTReader(GeometryFactory *gf){
	geometryFactory=gf;
	precisionModel=gf->getPrecisionModel();
}
WKTReader::~WKTReader(){
	delete geometryFactory;
	delete precisionModel;
}

Geometry* WKTReader::read(string wellKnownText){
	StringTokenizer *tokenizer=new StringTokenizer(wellKnownText);
	try {
		return readGeometryTaggedText(tokenizer);
	}
	catch (ParseException e) {
		throw new ParseException(e.toString());
	}
}

CoordinateList* WKTReader::getCoordinates(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return CoordinateListFactory::internalFactory->createCoordinateList();
	}
	CoordinateList *coordinates=CoordinateListFactory::internalFactory->createCoordinateList();
	Coordinate* externalCoordinate=new Coordinate();
	Coordinate* internalCoordinate=new Coordinate();
	externalCoordinate->x=getNextNumber(tokenizer);
	externalCoordinate->y=getNextNumber(tokenizer);
	precisionModel->toInternal(*externalCoordinate,internalCoordinate);
	coordinates->add(*(new Coordinate(*internalCoordinate)));
	nextToken=getNextCloserOrComma(tokenizer);
	while (nextToken==",") {
		externalCoordinate->x=getNextNumber(tokenizer);
		externalCoordinate->y=getNextNumber(tokenizer);
		precisionModel->toInternal(*externalCoordinate,internalCoordinate);
		coordinates->add(*(new Coordinate(*internalCoordinate)));
		nextToken=getNextCloserOrComma(tokenizer);
	}
	delete externalCoordinate;
	delete internalCoordinate;
	return coordinates;
}

double WKTReader::getNextNumber(StringTokenizer *tokenizer) {
	int type=tokenizer->nextToken();
	switch(type){
		case StringTokenizer::TT_EOF:
			throw ParseException("Expected number but encountered end of stream");
		case StringTokenizer::TT_EOL:
			throw ParseException("Expected number but encountered end of line");
		case StringTokenizer::TT_NUMBER:
			return tokenizer->getNVal();
		case StringTokenizer::TT_WORD:
			throw ParseException("Expected number but encountered word",tokenizer->getSVal());
		case '(':
			throw ParseException("Expected number but encountered '('");
		case ')':
			throw ParseException("Expected number but encountered ')'");
		case ',':
			throw ParseException("Expected number but encountered ','");
	}
	Assert::shouldNeverReachHere("Encountered unexpected StreamTokenizer type");
	return 0;
}

string WKTReader::getNextEmptyOrOpener(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord=="EMPTY" || nextWord=="(") {
		return nextWord;
	}
	throw ParseException("Expected 'EMPTY' or '(' but encountered ",nextWord);
}

string WKTReader::getNextCloserOrComma(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord=="," || nextWord==")") {
		return nextWord;
	}
	throw ParseException("Expected ')' or ',' but encountered",nextWord);
}

string WKTReader::getNextCloser(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord==")") {
		return nextWord;
	}
	throw ParseException("Expected ')' but encountered",nextWord);
}

string WKTReader::getNextWord(StringTokenizer *tokenizer) {
	int type=tokenizer->nextToken();
	switch(type){
		case StringTokenizer::TT_EOF:
			throw ParseException("Expected word but encountered end of stream");
		case StringTokenizer::TT_EOL:
			throw ParseException("Expected word but encountered end of line");
		case StringTokenizer::TT_NUMBER:
			throw ParseException("Expected word but encountered number", tokenizer->getNVal());
		case StringTokenizer::TT_WORD:
			return tokenizer->getSVal();
		case '(':
			return "(";
		case ')':
			return ")";
		case ',':
			return ",";
	}
	Assert::shouldNeverReachHere("Encountered unexpected StreamTokenizer type");
	return "";
}

Geometry* WKTReader::readGeometryTaggedText(StringTokenizer *tokenizer) {
	string type = getNextWord(tokenizer);
	if (type=="POINT") {
		return readPointText(tokenizer);
	} else if (type=="LINESTRING") {
		return readLineStringText(tokenizer);
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
	throw ParseException("Unknown type",type);
}

Point* WKTReader::readPointText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createPoint(Coordinate::getNull());
	}
	double x=getNextNumber(tokenizer);
	double y=getNextNumber(tokenizer);
	Coordinate* externalCoordinate=new Coordinate(x, y);
	Coordinate* internalCoordinate=new Coordinate();
	precisionModel->toInternal(*externalCoordinate,internalCoordinate);
	getNextCloser(tokenizer);
	return geometryFactory->createPoint(*internalCoordinate);
}

LineString* WKTReader::readLineStringText(StringTokenizer *tokenizer) {
	return geometryFactory->createLineString(getCoordinates(tokenizer));
}

LinearRing* WKTReader::readLinearRingText(StringTokenizer *tokenizer) {
	return geometryFactory->createLinearRing(getCoordinates(tokenizer));
}

MultiPoint* WKTReader::readMultiPointText(StringTokenizer *tokenizer) {
	return geometryFactory->createMultiPoint(getCoordinates(tokenizer));
}

Polygon* WKTReader::readPolygonText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createPolygon(NULL,NULL);
	}
	vector<Geometry *> *holes=new vector<Geometry *>();
	LinearRing *shell=new LinearRing(*readLinearRingText(tokenizer));
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		LinearRing *hole=new LinearRing(*readLinearRingText(tokenizer));
		holes->push_back(hole);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	return geometryFactory->createPolygon(shell,holes);
}

MultiLineString* WKTReader::readMultiLineStringText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createMultiLineString(NULL);
	}
	vector<Geometry *> *lineStrings=new vector<Geometry *>();
	LineString *lineString=new LineString(*readLineStringText(tokenizer));
	lineStrings->push_back(lineString);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		LineString *lineString=new LineString(*readLineStringText(tokenizer));
		lineStrings->push_back(lineString);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	return geometryFactory->createMultiLineString(lineStrings);
}

MultiPolygon* WKTReader::readMultiPolygonText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createMultiPolygon(NULL);
	}
	vector<Geometry *> *polygons=new vector<Geometry *>();
	Polygon *polygon=new Polygon(*readPolygonText(tokenizer));
	polygons->push_back(polygon);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		Polygon *polygon=new Polygon(*readPolygonText(tokenizer));
		polygons->push_back(polygon);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	return geometryFactory->createMultiPolygon(polygons);
}

GeometryCollection* WKTReader::readGeometryCollectionText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createGeometryCollection(NULL);
	}
	vector<Geometry *> *geoms=new vector<Geometry *>();
	Geometry *geom=new Geometry();
	geom=readGeometryTaggedText(tokenizer);
	geoms->push_back(geom);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		geom=readGeometryTaggedText(tokenizer);
		geoms->push_back(geom);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	return geometryFactory->createGeometryCollection(geoms);
}
