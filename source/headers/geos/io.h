/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_IO_H
#define GEOS_IO_H

#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <geos/platform.h>
#include <geos/geom.h>
#include <geos/util.h>

using namespace std;

namespace geos {

// A facility for use with WKB parser
typedef basic_stringstream<byte> biostringstream;

// biostringstream output
inline ostream &operator<<(ostream &s, biostringstream &o) {

	ios_base::fmtflags fl = s.flags(); // take note of output stream flags

	// Set hex,uppercase,fill and width output stream flags
	s.setf(ios::uppercase);
	s.setf(ios::hex, ios::basefield);
	s.setf(ios::hex, ios::basefield);
	s.fill('0');

	long pos = o.tellg(); // take note of input stream get pointer
	o.seekg(0, ios::beg); // rewind

	byte each=0;
	while(o.read(&each, 1)) s<<setw(2)<<(int)each;

	o.clear(); // clear input stream eof flag
	o.seekg(pos); // reset input stream position
	s.setf(fl);  // reset output stream status
	return s;
}



/**
 * \class ParseException io.h geos.h
 * \brief Notifies a parsing error
 */
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
	//StringTokenizer();
	StringTokenizer(const string& txt);
	~StringTokenizer() {};
	int nextToken();
	int peekNextToken();
	double getNVal();
	string getSVal();
private:
	const string &str;
	string stok;
	double ntok;
	string::const_iterator iter;
};

/**
 * Constant values used by the WKB format
 */
namespace WKBConstants {
	const int wkbXDR = 0;
	const int wkbNDR = 1;
	const int wkbPoint = 1;
	const int wkbLineString = 2;
	const int wkbPolygon = 3;
	const int wkbMultiPoint = 4;
	const int wkbMultiLineString = 5;
	const int wkbMultiPolygon = 6;
	const int wkbGeometryCollection = 7;
};

/**
 * \class WKTReader io.h geos.h
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

	~WKTReader() {};

	/// Parse a WKT string returning a Geometry
	Geometry* read(const string &wellKnownText);

//	Geometry* read(Reader reader);	//Not implemented yet

protected:
	CoordinateSequence* getCoordinates(StringTokenizer *tokenizer);
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

/*
 * \class ByteOrderValues io.h geos.h
 * 
 * Methods to read and write primitive datatypes from/to byte
 * sequences, allowing the byte order to be specified
 * 
 * Similar to the standard Java <code>ByteBuffer</code> class.
 */
class ByteOrderValues {

public:
	/*final*/ static int ENDIAN_BIG;
	/*final*/ static int ENDIAN_LITTLE;

	static int getInt(const byte *buf, int byteOrder);
	static void putInt(int intValue, byte *buf, int byteOrder);

	static int64 getLong(const byte *buf, int byteOrder);
	static void putLong(int64 longValue, byte *buf, int byteOrder);

	static double getDouble(const byte *buf, int byteOrder);
	static void putDouble(double doubleValue, byte *buf, int byteOrder);

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
 * \class WKTWriter io.h geos.h
 *
 * \brief Outputs the textual representation of a Geometry.
 * See also WKTReader.
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

} // namespace geos

#include <geos/ByteOrderDataInStreamT.h>
#include <geos/WKBReaderT.h>
#include <geos/WKBWriterT.h>

#endif

/**********************************************************************
 * $Log$
 * Revision 1.10  2005/04/29 16:36:43  strk
 * Fixed biostringstream stream output.
 *
 * Revision 1.9  2005/04/29 15:34:21  strk
 * Typedef'ed biostringstream, preferred parameter for
 * WKB parser templates.
 * Added << operator for biostringstream.
 * Typedef'ed WKBWriter and WKBReader to be parametrized by
 * biostringstream.
 * Added WKBtest in doc/example.cpp
 *
 * Revision 1.8  2005/04/29 11:52:40  strk
 * Added new JTS interfaces for CoordinateSequence and factories,
 * removed example implementations to reduce maintainance costs.
 * Added first implementation of WKBWriter, made ByteOrderDataInStream
 * a template class.
 *
 * Revision 1.7  2005/04/20 17:22:47  strk
 * Added initial implementation of WKBReaderT and ByteOrderDataInStreamT
 * class templates and ByteOrderValues class.
 * Work is unfinished as WKBReader requires new interface of CoordinateSequence
 * taking higher dimensions into account.
 *
 * Revision 1.6  2005/04/14 11:49:01  strk
 * Applied slightly modified patch by Cheng Shan to speedup WKT parsing.
 *
 * Revision 1.5  2004/07/19 10:33:12  strk
 * Class documentation changed to report geos.h as WKT writer/parser header file
 *
 * Revision 1.4  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/07 10:29:54  strk
 * Adjusted exceptions documentation.
 *
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

