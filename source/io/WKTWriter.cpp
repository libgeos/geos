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
 * Revision 1.14  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/io.h"
#include "../headers/util.h"
#include <typeinfo>
#include "stdio.h"

namespace geos {

WKTWriter::WKTWriter() {
	isFormatted=false;
	level=0;
	formatter="%f";
}

WKTWriter::~WKTWriter() {}

string WKTWriter::createFormatter(const PrecisionModel* precisionModel) {
	// the default number of decimal places is 16, which is sufficient
	// to accomodate the maximum precision of a double.
	int decimalPlaces = 16;
	if (!precisionModel->isFloating()) {
		decimalPlaces = 1 + (int) ceil(log(precisionModel->getScale())/log(10.0));
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

string WKTWriter::write(const Geometry *geometry) {
	Writer *sw=new Writer();
//	try {
		writeFormatted(geometry,false,sw);
//	} catch (IOException ex) {
//		Assert::shouldNeverReachHere();
//	}
	string res=sw->toString();
	delete sw;
	return res;
}

void WKTWriter::write(const Geometry *geometry, Writer *writer) {
	writeFormatted(geometry, false, writer);
}

string WKTWriter::writeFormatted(const Geometry *geometry) {
	Writer *sw=new Writer();
//	try {
		writeFormatted(geometry, true, sw);
//	}catch (IOException ex) {
//		Assert::shouldNeverReachHere();
//	}
	return sw->toString();
}

void WKTWriter::writeFormatted(const Geometry *geometry, Writer *writer) {
	writeFormatted(geometry, true, writer);
}

void WKTWriter::writeFormatted(const Geometry *geometry, bool isFormatted, Writer *writer) {
	this->isFormatted=isFormatted;
	formatter=createFormatter(geometry->getPrecisionModel());
	appendGeometryTaggedText(geometry, 0, writer);
}

void WKTWriter::appendGeometryTaggedText(const Geometry *geometry, int level, Writer *writer) {
	indent(level, writer);
	if (typeid(*geometry)==typeid(Point)) {
		Point* point=(Point*)geometry;
		appendPointTaggedText(point->getCoordinate(),level,writer,point->getPrecisionModel());
	} else if (typeid(*geometry)==typeid(LineString)) {
		appendLineStringTaggedText((LineString*)geometry, level, writer);
	} else if (typeid(*geometry)==typeid(LinearRing)) {
		appendLinearRingTaggedText((LinearRing*)geometry, level, writer);
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
//		Assert.shouldNeverReachHere("Unsupported Geometry implementation:" + geometry->getClass());
		Assert::shouldNeverReachHere("Unsupported Geometry implementation");
	}
}

void
WKTWriter::appendPointTaggedText(const Coordinate* coordinate, int level,
		Writer *writer, const PrecisionModel* precisionModel) {
	writer->write("POINT ");
	appendPointText(coordinate, level, writer, precisionModel);
}

void WKTWriter::appendLineStringTaggedText(const LineString *lineString, int level, Writer *writer) {
	writer->write("LINESTRING ");
	appendLineStringText(lineString, level, false, writer);
}

void WKTWriter::appendLinearRingTaggedText(const LinearRing *lineString, int level, Writer *writer) {
	writer->write("LINEARRING ");
	appendLineStringText((LineString *)lineString, level, false, writer);
}

void WKTWriter::appendPolygonTaggedText(const Polygon *polygon, int level, Writer *writer) {
	writer->write("POLYGON ");
	appendPolygonText(polygon, level, false, writer);
}

void WKTWriter::appendMultiPointTaggedText(const MultiPoint *multipoint, int level, Writer *writer) {
	writer->write("MULTIPOINT ");
	appendMultiPointText(multipoint, level, writer);
}

void WKTWriter::appendMultiLineStringTaggedText(const MultiLineString *multiLineString, int level,Writer *writer) {
	writer->write("MULTILINESTRING ");
	appendMultiLineStringText(multiLineString, level, false, writer);
}

void WKTWriter::appendMultiPolygonTaggedText(const MultiPolygon *multiPolygon, int level, Writer *writer) {
	writer->write("MULTIPOLYGON ");
	appendMultiPolygonText(multiPolygon, level, writer);
}

void WKTWriter::appendGeometryCollectionTaggedText(const GeometryCollection *geometryCollection, int level,Writer *writer) {
	writer->write("GEOMETRYCOLLECTION ");
	appendGeometryCollectionText(geometryCollection, level, writer);
}

void
WKTWriter::appendPointText(const Coordinate* coordinate, int level,
		Writer *writer,const PrecisionModel* precisionModel) {
	if (coordinate==NULL) {
		writer->write("EMPTY");
	} else {
		writer->write("(");
		appendCoordinate(coordinate, writer, precisionModel);
		writer->write(")");
	}
}

void WKTWriter::appendCoordinate(const Coordinate* coordinate, Writer *writer, const PrecisionModel* precisionModel) {
	Coordinate* externalCoordinate=new Coordinate();
	precisionModel->toExternal(*coordinate, externalCoordinate);
	string out="";
	out+=writeNumber(externalCoordinate->x);
	out+=" ";
	out+=writeNumber(externalCoordinate->y);
	writer->write(out);
	delete externalCoordinate;
}

string WKTWriter::writeNumber(double d) {
	string out="";
	char buffer[255];
	sprintf(buffer,formatter.c_str(),d);
	out.append(buffer);
	out.append("");
	return out;
}

void WKTWriter::appendLineStringText(const LineString *lineString, int level, bool doIndent, Writer *writer) {
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
			appendCoordinate(&(lineString->getCoordinateN(i)), writer, lineString->getPrecisionModel());
		}
		writer->write(")");
	}
}

void WKTWriter::appendPolygonText(const Polygon *polygon, int level, bool indentFirst, Writer *writer) {
	if (polygon->isEmpty()) {
		writer->write("EMPTY");
	} else {
		if (indentFirst) indent(level, writer);
		writer->write("(");
		appendLineStringText(polygon->getExteriorRing(), level, false, writer);
		for (int i=0; i<polygon->getNumInteriorRing(); i++) {
			writer->write(", ");
			const LineString *ls=polygon->getInteriorRingN(i);
			appendLineStringText(polygon->getInteriorRingN(i), level + 1, true, writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiPointText(const MultiPoint *multiPoint, int level, Writer *writer) {
	if (multiPoint->isEmpty()) {
		writer->write("EMPTY");
	} else {
		writer->write("(");
		for (int i=0; i<multiPoint->getNumGeometries(); i++) {
			if (i > 0) {
				writer->write(", ");
			}
			appendCoordinate(((Point* )multiPoint->getGeometryN(i))->getCoordinate(), writer,
							  multiPoint->getPrecisionModel());
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiLineStringText(const MultiLineString *multiLineString, int level, bool indentFirst,
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
			appendLineStringText((LineString *) multiLineString->getGeometryN(i), level2, doIndent, writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendMultiPolygonText(const MultiPolygon *multiPolygon, int level, Writer *writer) {
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
			appendPolygonText((Polygon *) multiPolygon->getGeometryN(i), level2, doIndent, writer);
		}
		writer->write(")");
	}
}

void WKTWriter::appendGeometryCollectionText(const GeometryCollection *geometryCollection, int level, Writer *writer) {
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
			appendGeometryTaggedText(geometryCollection->getGeometryN(i),level2,writer);
		}
		writer->write(")");
	}
}

void WKTWriter::indent(int level, Writer *writer) {
	if (!isFormatted || level<=0) return;
	writer->write("\n");
	writer->write(stringOfChar(' ', INDENT * level));
}
}

