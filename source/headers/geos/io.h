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

#ifndef GEOS_IO_H
#define GEOS_IO_H

#include <geos/platform.h>
#include <geos/util.h>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

// Forward declarations
namespace geos {
	namespace geom { 
		class Coordinate;
		class CoordinateSequence;
		class Geometry;
		class Point;
		class LinearRing;
		class LineString;
		class Polygon;
		class MultiPoint;
		class MultiLineString;
		class MultiPolygon;
		class GeometryCollection;
		class GeometryFactory;
		class PrecisionModel;
	}
}

// this headers needs to be splitted, anyway
using namespace geos::geom;

namespace geos {


/// Contains the interfaces for converting JTS objects to and from other formats.
//
/// The Java Topology Suite (JTS) is a Java API that implements a core
/// set of spatial data operations usin g an explicit precision model
/// and robust geometric algorithms. JTS is intended to be used in the
/// devel opment of applications that support the validation, cleaning,
/// integration and querying of spatial data sets.
///
/// JTS attempts to implement the OpenGIS Simple Features Specification
/// (SFS) as accurately as possible.  In some cases the SFS is unclear
/// or omits a specification; in this case JTS attempts to choose a reaso
/// nable and consistent alternative.  Differences from and elaborations
/// of the SFS are documented in this specification.
///
/// <h2>Package Specification</h2>
///
/// <ul>
/// <li>Java Topology Suite Technical Specifications
/// <li><A HREF="http://www.opengis.org/techno/specs.htm">
///   OpenGIS Simple Features Specification for SQL</A>
/// </ul>
///
namespace io { // geos.io

/// An unsigned char
typedef unsigned char byte;

/**
 * \class ParseException io.h geos.h
 * \brief Notifies a parsing error
 */
class ParseException: public util::GEOSException {
public:
	ParseException();
	ParseException(std::string msg);
	ParseException(std::string msg, std::string var);
	ParseException(std::string msg, double num);
	~ParseException() throw() {};
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
	StringTokenizer(const std::string& txt);
	~StringTokenizer() {};
	int nextToken();
	int peekNextToken();
	double getNVal();
	std::string getSVal();
private:
	const std::string &str;
	std::string stok;
	double ntok;
	std::string::const_iterator iter;
};

/// Constant values used by the WKB format
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
}

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
	WKTReader(const geom::GeometryFactory *gf);

	~WKTReader() {};

	/// Parse a WKT string returning a Geometry
	Geometry* read(const std::string &wellKnownText);

//	Geometry* read(Reader& reader);	//Not implemented yet

protected:
	CoordinateSequence* getCoordinates(StringTokenizer *tokenizer);
	double getNextNumber(StringTokenizer *tokenizer);
	std::string getNextEmptyOrOpener(StringTokenizer *tokenizer);
	std::string getNextCloserOrComma(StringTokenizer *tokenizer);
	std::string getNextCloser(StringTokenizer *tokenizer);
	std::string getNextWord(StringTokenizer *tokenizer);
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
	void write(std::string txt);
	std::string toString();
private:
	std::string str;
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
	std::string write(const Geometry *geometry);

	// Send Geometry's WKT to the given Writer
	void write(const Geometry *geometry, Writer *writer);

	std::string writeFormatted(const Geometry *geometry);

	void writeFormatted(const Geometry *geometry, Writer *writer);

	/**
	 * Generates the WKT for a N-point <code>LineString</code>.
	 *
	 * @param seq the sequence to outpout
	 *
	 * @return the WKT
	 */
	static std::string toLineString(const CoordinateSequence& seq); 

	/**
	 * Generates the WKT for a 2-point <code>LineString</code>.
	 *
	 * @param p0 the first coordinate
	 * @param p1 the second coordinate
	 *
	 * @return the WKT
	 */
	static std::string toLineString(const Coordinate& p0, const Coordinate& p1);

	/**
	 * Generates the WKT for a <code>Point</code>.
	 *
	 * @param p0 the point coordinate
	 *
	 * @return the WKT
	 */
	static std::string toPoint(const Coordinate& p0);
 

protected:
	std::string formatter;
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
	std::string writeNumber(double d);
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
	static std::string createFormatter(const PrecisionModel* precisionModel);
	bool isFormatted;
	int level;
	void writeFormatted(const Geometry *geometry, bool isFormatted, Writer *writer);
	void indent(int level, Writer *writer);
};

/*
 * \class ByteOrderDataInStream io.h geos.h
 * 
 * Allows reading an stream of primitive datatypes from an underlying
 * istream, with the representation being in either common byte ordering.
 *
 */
class ByteOrderDataInStream {

public:

	ByteOrderDataInStream(std::istream *s=NULL):
		byteOrder(getMachineByteOrder()),
		stream(s) {}
	~ByteOrderDataInStream() {}

	/**
	 * Allows a single ByteOrderDataInStreamT to be reused
	 * on multiple istream.
	 */
	void setInStream(std::istream *s) { stream=s; }
	void setOrder(int order) { byteOrder=order; }

	byte readByte() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 1);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return buf[0];
	}

	int readInt() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 4);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return ByteOrderValues::getInt(buf, byteOrder);
	}

	long readLong() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 8);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return ByteOrderValues::getLong(buf, byteOrder);
	}

	double readDouble() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 8);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return ByteOrderValues::getDouble(buf, byteOrder);
	}

private:
	int byteOrder;
	std::istream *stream;

	// buffers to hold primitive datatypes
	byte buf[8];

};

/**
 * \class WKBReader io.h geos.h
 *
 * \brief Reads a Geometry from Well-Known Binary format.
 *
 * This class is designed to support reuse of a single instance to read
 * multiple geometries. This class is not thread-safe; each thread should
 * create its own instance.
 *
 * The Well-known Binary format is defined in the <A
 * HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 * Specification for SQL</A>.
 * This implementation supports the extended WKB standard which allows
 * representing 3-dimensional coordinates.
 *
 */
class WKBReader {

public:

	WKBReader(const GeometryFactory &f): factory(f) {};

	/**
	 * \brief Reads a Geometry from an istream.
	 *
	 * @param is the stream to read from 
	 * @return the Geometry read
	 * @throws IOException
	 * @throws ParseException
	 */
	Geometry *read(std::istream &is);
		// throws IOException, ParseException

	/**
	 * \brief Reads a Geometry from an istream in hex format.
	 *
	 * @param is the stream to read from 
	 * @return the Geometry read
	 * @throws IOException
	 * @throws ParseException
	 */
	Geometry *readHEX(std::istream &is);
		// throws IOException, ParseException

	/**
	 * \brief Print WKB in HEX form to out stream
	 *
	 * @param is is the stream to read from
	 * @param os is the stream to write to
	 */
	static std::ostream &printHEX(std::istream &is, std::ostream &os);
 
private:

	static std::string BAD_GEOM_TYPE_MSG;

	const GeometryFactory &factory;

	// for now support the WKB standard only - may be generalized later
	unsigned int inputDimension;

	ByteOrderDataInStream dis;

	std::vector<double> ordValues;

	Geometry *readGeometry();
		// throws IOException, ParseException

	Point *readPoint();
		// throws IOException

	LineString *readLineString();
		// throws IOException

	LinearRing *readLinearRing();
		// throws IOException

	Polygon *readPolygon();
		// throws IOException

	MultiPoint *readMultiPoint();
		// throws IOException, ParseException

	MultiLineString *readMultiLineString();
		// throws IOException, ParseException

	MultiPolygon *readMultiPolygon();
		// throws IOException, ParseException

	GeometryCollection *readGeometryCollection();
		// throws IOException, ParseException

	CoordinateSequence *readCoordinateSequence(int); // throws IOException

	void readCoordinate(); // throws IOException


};

/**
 *
 * \class WKBWriter io.h geos.h
 *
 * \brief Writes a Geometry into Well-Known Binary format.
 *
 * The WKB format is specified in the OGC Simple Features for SQL specification.
 * This implementation supports the extended WKB standard for representing
 * 3-dimensional coordinates.  The presence of 3D coordinates is signified
 * by setting the high bit of the wkbType word.
 *
 * Empty Points cannot be represented in WKB; an
 * IllegalArgumentException will be thrown if one is
 * written. The WKB specification does not support representing LinearRing
 * they will be written as LineString
 * 
 * This class is designed to support reuse of a single instance to read multiple
 * geometries. This class is not thread-safe; each thread should create its own
 * instance.
 *
 * @see WKBReader
 */
class WKBWriter {

public:

	WKBWriter(int dims=2, int bo=getMachineByteOrder());

	/**
	 * \brief Write a Geometry to an ostream.
	 *
	 * @param g the geometry to write
	 * @param os the output stream
	 * @throws IOException
	 */
	void write(const Geometry &g, std::ostream &os);
		// throws IOException, ParseException

	/**
	 * \brief Write a Geometry to an ostream in binary hex format.
	 *
	 * @param g the geometry to write
	 * @param os the output stream
	 * @throws IOException
	 */
	void writeHEX(const Geometry &g, std::ostream &os);
		// throws IOException, ParseException

private:

	int outputDimension;

	int byteOrder;

	std::ostream *outStream;

	byte buf[8];

	void writePoint(const Point &p);
		// throws IOException

	void writeLineString(const LineString &ls);
		// throws IOException

	void writePolygon(const Polygon &p);
		// throws IOException

	void writeGeometryCollection(const GeometryCollection &c, int wkbtype);
		// throws IOException, ParseException

	void writeCoordinateSequence(const CoordinateSequence &cs, bool sized);
		// throws IOException

	void writeCoordinate(const CoordinateSequence &cs, int idx, bool is3d);
		// throws IOException

	void writeGeometryType(int geometryType);
		// throws IOException

	void writeByteOrder();
		// throws IOException

	void writeInt(int intValue);
		// throws IOException

};


} // namespace geos.io
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.26  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.25  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.24  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.23  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.22  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.21  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.20  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
 * Revision 1.19  2006/01/15 21:19:06  strk
 * Added WKBWriter::writeHEX() and WKBReader::readHEX()
 *
 * Revision 1.18  2005/11/29 13:39:56  strk
 * Moved byte typedef from geom.h to io.h.
 * Removed useless commas at inlined funx end.
 * Changed CoordinateSequenceFactory::create(siz,dims) to use unsigned for dims.
 *
 * Revision 1.17  2005/10/19 12:59:35  strk
 * Added input stream checks in ByteOrderDataInStream, throwing ParseException on unexpected EOF
 *
 * Revision 1.16  2005/09/28 10:15:34  strk
 * WKBWriter ctor out-lined (will require changes in the near future, for higher dimensions support), added support for 3d WKB output
 *
 * Revision 1.15  2005/09/27 16:20:43  strk
 * Reverted previous change, fixed the bug by turning WKBReader.factory
 * into a reference rather then a real object. ABI still breaks, but API
 * at least is safe (we didn't release any WKB-aware package so breaking
 * ABI is not a big deal at this stage).
 *
 * Revision 1.14  2005/09/27 16:00:26  strk
 * Fixed bug in WKBReader destroying the GeometryFactory used in Geometry
 * construction. Changed it's definition to *require* a GeometryFactory
 * pointer parameter.
 *
 * Revision 1.13  2005/09/03 21:26:42  strk
 * Reworked WKB I/O to avoid use of templates and make better use of STL
 *
 * Revision 1.12  2005/07/11 12:17:47  strk
 * Cleaned up syntax
 *
 * Revision 1.11  2005/04/29 17:40:36  strk
 * Updated Doxygen documentation and some Copyright headers.
 *
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

