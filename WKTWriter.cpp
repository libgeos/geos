#include "io.h"
#include <typeinfo>

WKTWriter::WKTWriter() {
	isFormatted=false;
	level=0;
	formatter="%f";
}

WKTWriter::~WKTWriter() {}

string WKTWriter::createFormatter(PrecisionModel precisionModel) {
	// the default number of decimal places is 16, which is sufficient
	// to accomodate the maximum precision of a double.
	int decimalPlaces = 16;
	if (!precisionModel.isFloating()) {
		decimalPlaces = 1 + (int) ceil(log(precisionModel.getScale())/log(10));
	}
	string fmt="%.";
	char buffer[255];
	sprintf(buffer,"%i",decimalPlaces);
	fmt.append(buffer);
	fmt.append("f");
	return fmt;
}

string WKTWriter::stringOfChar(char ch, int count) {
	string str="";
	for (int i=0;i<count;i++)
		str+=ch;
	return str;
}

//!!! External dependency
string WKTWriter::write(Geometry *geometry) {
	Writer sw;
//	try {
		writeFormatted(geometry,false,&sw);
//	} catch (IOException ex) {
//		Assert.shouldNeverReachHere();
//	}
	return sw.toString();
}

void WKTWriter::write(Geometry *geometry, Writer *writer) {
	writeFormatted(geometry, false, writer);
}

//!!! External dependency
string WKTWriter::writeFormatted(Geometry *geometry) {
	Writer sw;
//	try {
		writeFormatted(geometry, true, &sw);
//	}catch (IOException ex) {
//		Assert.shouldNeverReachHere();
//	}
	return sw.toString();
}

void WKTWriter::writeFormatted(Geometry *geometry, Writer *writer) {
	writeFormatted(geometry, true, writer);
}

void WKTWriter::writeFormatted(Geometry *geometry, bool isFormatted, Writer *writer) {
	this->isFormatted=isFormatted;
	formatter=createFormatter(geometry->getPrecisionModel());
	appendGeometryTaggedText(geometry, 0, writer);
}

//!!! External dependency
void WKTWriter::appendGeometryTaggedText(Geometry *geometry, int level, Writer *writer) {
	indent(level, writer);
	if (typeid(*geometry)==typeid(Point)) {
		Point point(*(Point*)geometry);
		appendPointTaggedText(point.getCoordinate(),level,writer, point.getPrecisionModel());
	} else if (typeid(*geometry)==typeid(LineString)) {
		appendLineStringTaggedText((LineString*)geometry, level, writer);
	} else if (typeid(*geometry)==typeid(Polygon)) {
		appendPolygonTaggedText((Polygon*)geometry, level, writer);
	} else if (typeid(*geometry)==typeid(MultiPoint)) {
		appendMultiPointTaggedText((MultiPoint*)geometry, level, writer);
	} else if (typeid(*geometry)==typeid(MultiLineString)) {
		appendMultiLineStringTaggedText((MultiLineString*)geometry, level, writer);
	} else if (typeid(*geometry)==typeid(MultiPolygon)) {
		appendMultiPolygonTaggedText((MultiPolygon*)geometry, level, writer);
	} else if (typeid(*geometry)==typeid(GeometryCollection)) {
		appendGeometryCollectionTaggedText((GeometryCollection*)geometry, level, writer);
	} else {
//!!! External dependency
//		Assert.shouldNeverReachHere("Unsupported Geometry implementation:" + geometry->getClass());
	}
}

void WKTWriter::appendPointTaggedText(Coordinate coordinate, int level, Writer *writer,
									  PrecisionModel precisionModel) {
	writer->write("POINT ");
	appendPointText(coordinate, level, writer, precisionModel);
}

void WKTWriter::appendLineStringTaggedText(LineString *lineString, int level, Writer *writer) {
	writer->write("LINESTRING ");
	appendLineStringText(lineString, level, false, writer);
}

void WKTWriter::appendPolygonTaggedText(Polygon *polygon, int level, Writer *writer) {
	writer->write("POLYGON ");
	appendPolygonText(polygon, level, false, writer);
}

void WKTWriter::appendMultiPointTaggedText(MultiPoint *multipoint, int level, Writer *writer) {
	writer->write("MULTIPOINT ");
	appendMultiPointText(multipoint, level, writer);
}

void WKTWriter::appendMultiLineStringTaggedText(MultiLineString *multiLineString, int level,Writer *writer) {
	writer->write("MULTILINESTRING ");
	appendMultiLineStringText(multiLineString, level, false, writer);
}

void WKTWriter::appendMultiPolygonTaggedText(MultiPolygon *multiPolygon, int level, Writer *writer) {
	writer->write("MULTIPOLYGON ");
	appendMultiPolygonText(multiPolygon, level, writer);
}

void WKTWriter::appendGeometryCollectionTaggedText(GeometryCollection *geometryCollection, int level,Writer *writer) {
	writer->write("GEOMETRYCOLLECTION ");
	appendGeometryCollectionText(geometryCollection, level, writer);
}

void WKTWriter::appendPointText(Coordinate coordinate, int level, Writer *writer,PrecisionModel precisionModel) {
	if (coordinate==Coordinate::getNull()) {
		writer->write("EMPTY");
	} else {
		writer->write("(");
		appendCoordinate(coordinate, writer, precisionModel);
		writer->write(")");
	}
}

void WKTWriter::appendCoordinate(Coordinate coordinate, Writer *writer, PrecisionModel precisionModel) {
	Coordinate externalCoordinate;
	precisionModel.toExternal(coordinate, &externalCoordinate);
	string out="";
	out+=writeNumber(externalCoordinate.x);
	out+=" ";
	out+=writeNumber(externalCoordinate.y);
	writer->write(out);
}

string WKTWriter::writeNumber(double d) {
	string out="";
	char buffer[255];
	sprintf(buffer,formatter.c_str(),d);
	out.append(buffer);
	out.append("");
	return out;
}

void WKTWriter::appendLineStringText(LineString *lineString, int level, bool doIndent, Writer *writer) {
	if (lineString->isEmpty()) {
		writer->write("EMPTY");
	} else {
		if (doIndent) indent(level, writer);
		writer->write("(");
		for(int i=0;i<lineString->getNumPoints();i++) {
			if (i>0) {
				writer->write(", ");
				if (i%10==0) indent(level + 2, writer);
			}
			appendCoordinate(lineString->getCoordinateN(i), writer, lineString->getPrecisionModel());
		}
		writer->write(")");
	}
}

void WKTWriter::appendPolygonText(Polygon *polygon, int level, bool indentFirst, Writer *writer) {
	if (polygon->isEmpty()) {
		writer->write("EMPTY");
	} else {
		if (indentFirst) indent(level, writer);
		writer->write("(");
		appendLineStringText(&(polygon->getExteriorRing()), level, false, writer);
		for (int i=0; i<polygon->getNumInteriorRing(); i++) {
			writer->write(", ");
			appendLineStringText(&(polygon->getInteriorRingN(i)), level + 1, true, writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiPointText(MultiPoint *multiPoint, int level, Writer *writer) {
	if (multiPoint->isEmpty()) {
		writer->write("EMPTY");
	} else {
		writer->write("(");
		for (int i=0; i<multiPoint->getNumGeometries(); i++) {
			if (i > 0) {
				writer->write(", ");
			}
			appendCoordinate(((Point* )&(multiPoint->getGeometryN(i)))->getCoordinate(), writer,
							  multiPoint->getPrecisionModel());
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiLineStringText(MultiLineString *multiLineString, int level, bool indentFirst,
											Writer *writer) {
	if (multiLineString->isEmpty()) {
		writer->write("EMPTY");
	} else {
		int level2=level;
		bool doIndent=indentFirst;
		writer->write("(");
		for (int i=0; i<multiLineString->getNumGeometries();i++) {
			if (i>0) {
				writer->write(", ");
				level2=level+1;
				doIndent=true;
			}
			appendLineStringText((LineString *) &(multiLineString->getGeometryN(i)), level2, doIndent, writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiPolygonText(MultiPolygon *multiPolygon, int level, Writer *writer) {
	if (multiPolygon->isEmpty()) {
		writer->write("EMPTY");
	} else {
		int level2=level;
		bool doIndent=false;
		writer->write("(");
		for (int i=0; i<multiPolygon->getNumGeometries();i++) {
			if (i>0) {
				writer->write(", ");
				level2=level+1;
				doIndent=true;
			}
			appendPolygonText((Polygon *) &(multiPolygon->getGeometryN(i)), level2, doIndent, writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendGeometryCollectionText(GeometryCollection *geometryCollection, int level, Writer *writer) {
	if (geometryCollection->isEmpty()) {
		writer->write("EMPTY");
	} else {
		int level2=level;
		writer->write("(");
		for (int i=0; i<geometryCollection->getNumGeometries();i++) {
			if (i>0) {
				writer->write(", ");
				level2=level+1;
			}
			appendGeometryTaggedText(&(geometryCollection->getGeometryN(i)),level2,writer);
		}
		writer->write(")");
	}
}

void WKTWriter::indent(int level, Writer *writer) {
	if (!isFormatted || level<=0) return;
	writer->write("\n");
	writer->write(stringOfChar(' ', INDENT * level));
}
