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

#ifndef GEOS_IO_WKTREADER_H
#define GEOS_IO_WKTREADER_H

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <string>

// Forward declarations
namespace geos {
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

	} // namespace geom
} // namespace geos


namespace geos {
namespace io {

class StringTokenizer;

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
	geom::Coordinate* getPreciseCoordinate(io::StringTokenizer *tokenizer);
	bool isNumberNext(io::StringTokenizer *tokenizer);
};

} // namespace io
} // namespace geos

#ifdef USE_INLINE
# include <geos/io/WKTReader.inl>
#endif

#endif // #ifndef GEOS_IO_WKTREADER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 18:18:15  strk
 * io.h header split
 *
 **********************************************************************/
