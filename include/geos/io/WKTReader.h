/**********************************************************************
 * $Id: WKTReader.h 2990 2010-05-18 19:18:54Z warmerdam $
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
 **********************************************************************
 *
 * Last port: io/WKTReader.java rev. 1.1 (JTS-1.7)
 *
 **********************************************************************/

#ifndef GEOS_IO_WKTREADER_H
#define GEOS_IO_WKTREADER_H

#include <geos/export.h>

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <string>

// Forward declarations
namespace geos {
	namespace io {
		class StringTokenizer;
	}
	namespace geom {

		class Coordinate;
		class Geometry;
		class GeometryCollection;
		class Point;
		class LineString;
		class LinearRing;
		class Polygon;
		class MultiPoint;
		class MultiLineString;
		class MultiPolygon;
		class PrecisionModel;
	} 
} 


namespace geos {
namespace io {

/**
 * \class WKTReader io.h geos.h
 * \brief WKT parser class; see also WKTWriter.
 */
class GEOS_DLL WKTReader {
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

	/**
	 * \brief Inizialize parser with default GeometryFactory.
	 *
	 */
	WKTReader();

	~WKTReader();

	/// Parse a WKT string returning a Geometry
	geom::Geometry* read(const std::string &wellKnownText);

//	Geometry* read(Reader& reader);	//Not implemented yet

protected:
	geom::CoordinateSequence* getCoordinates(io::StringTokenizer *tokenizer);
	double getNextNumber(io::StringTokenizer *tokenizer);
	std::string getNextEmptyOrOpener(io::StringTokenizer *tokenizer);
	std::string getNextCloserOrComma(io::StringTokenizer *tokenizer);
	std::string getNextCloser(io::StringTokenizer *tokenizer);
	std::string getNextWord(io::StringTokenizer *tokenizer);
	geom::Geometry* readGeometryTaggedText(io::StringTokenizer *tokenizer);
	geom::Point* readPointText(io::StringTokenizer *tokenizer);
	geom::LineString* readLineStringText(io::StringTokenizer *tokenizer);
	geom::LinearRing* readLinearRingText(io::StringTokenizer *tokenizer);
	geom::MultiPoint* readMultiPointText(io::StringTokenizer *tokenizer);
	geom::Polygon* readPolygonText(io::StringTokenizer *tokenizer);
	geom::MultiLineString* readMultiLineStringText(io::StringTokenizer *tokenizer);
	geom::MultiPolygon* readMultiPolygonText(io::StringTokenizer *tokenizer);
	geom::GeometryCollection* readGeometryCollectionText(io::StringTokenizer *tokenizer);
private:
	const geom::GeometryFactory *geometryFactory;
	const geom::PrecisionModel *precisionModel;

	void getPreciseCoordinate(io::StringTokenizer *tokenizer, geom::Coordinate&, std::size_t &dim );

	bool isNumberNext(io::StringTokenizer *tokenizer);
};

} // namespace io
} // namespace geos

#ifdef GEOS_INLINE
# include <geos/io/WKTReader.inl>
#endif

#endif // #ifndef GEOS_IO_WKTREADER_H

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/04/10 13:40:14  strk
 * Added default ctor for WKTReader (using GeometryFactory's default instance)
 *
 * Revision 1.5  2006/04/10 12:05:35  strk
 * Added inline-replicator implementation files to make sure
 * functions in .inl files are still available out-of-line.
 * A side effect is this should fix MingW build.
 *
 * Revision 1.4  2006/03/30 09:26:36  strk
 * minor cleanup
 *
 * Revision 1.3  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.2  2006/03/22 16:58:35  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.1  2006/03/20 18:18:15  strk
 * io.h header split
 *
 **********************************************************************/
