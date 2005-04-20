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
 **********************************************************************/

#ifndef _WKBREADERT_H
#define _WKBREADERT_H 1

#include <geos/geom.h>
#include <geos/io.h>
#include <geos/ByteOrderDataInStreamT.h>

namespace geos {

/*
 * \class WKBReaderT io.h geos.h
 * \brief WKB parser class; see also WKBWriter.
 *
 * Reads a Geometry from a byte stream in Well-Known Binary format.
 * Supports use of an InStream, which allows easy use
 * with arbitary byte stream sources.
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
 * This is a template class.
 * 
 */
template <class InStream>
class WKBReaderT {

private:

	static string BAD_GEOM_TYPE_MSG;

	const GeometryFactory factory;

	// for now support the WKB standard only - may be generalized later
	int inputDimension;

	ByteOrderDataInStreamT<InStream> dis;

	vector<double> ordValues;

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


public:

	WKBReaderT() {};
	WKBReaderT(const GeometryFactory &f): factory(f) {};

	/**
	 * Reads a Geometry from an InStream.
	 *
	 * @param is the stream to read from 
	 * @return the Geometry read
	 * @throws IOException
	 * @throws ParseException
	 */
	Geometry *read(InStream &is);
		// throws IOException, ParseException
 
};
template<class InStream> string WKBReaderT<InStream>::BAD_GEOM_TYPE_MSG = "bad geometry type encountered in ";

template<class InStream> Geometry *
WKBReaderT<InStream>::read(InStream &is)
{
	dis.setInStream(is);
	inputDimension = 2; // handle 2d only for now

	// only allocate ordValues buffer if necessary
	if ( ordValues.size() < inputDimension )
		ordValues.resize(inputDimension);

	return readGeometry();
}

template<class InStream> Geometry *
WKBReaderT<InStream>::readGeometry()
{
	// determine byte order
	byte byteOrder = dis.readByte();
	// default is big endian
	if (byteOrder == WKBConstants::wkbNDR)
		dis.setOrder(ByteOrderValues::ENDIAN_LITTLE);

	int typeInt = dis.readInt();
	int geometryType = typeInt & 0xff;
	bool hasZ = (typeInt & 0x80000000) != 0;
	if (hasZ) inputDimension = 3;

	switch (geometryType) {
		case WKBConstants::wkbPoint :
			return readPoint();
		case WKBConstants::wkbLineString :
			return readLineString();
		case WKBConstants::wkbPolygon :
			return readPolygon();
		case WKBConstants::wkbMultiPoint :
			return readMultiPoint();
		case WKBConstants::wkbMultiLineString :
			return readMultiLineString();
		case WKBConstants::wkbMultiPolygon :
			return readMultiPolygon();
		case WKBConstants::wkbGeometryCollection :
			return readGeometryCollection();
	}
	throw new ParseException("Unknown WKB type " + geometryType);
}

template<class InStream> Point *
WKBReaderT<InStream>::readPoint()
{
	readCoordinate();
	return factory.createPoint(Coordinate(ordValues[0], ordValues[1]));
}

template<class InStream> LineString *
WKBReaderT<InStream>::readLineString()
{
	int size = dis.readInt();
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLineString(pts);
}

template<class InStream> LinearRing *
WKBReaderT<InStream>::readLinearRing()
{
	int size = dis.readInt();
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLinearRing(pts);
}

template<class InStream> Polygon *
WKBReaderT<InStream>::readPolygon()
{
	int numRings = dis.readInt();
	LinearRing *shell = readLinearRing();

	vector<Geometry *>*holes=NULL;
	if ( numRings > 1 )
	{
		try {
			holes = new vector<Geometry *>(numRings-1);
			for (int i=0; i<numRings-1; i++)
				(*holes)[i] = (Geometry *)readLinearRing();
		} catch (...) {
			for (int i=0; i<holes->size(); i++)
				delete (*holes)[i];
			delete holes;
			delete shell;
			throw;
		}
	}
	return factory.createPolygon(shell, holes);
}

template<class InStream> MultiPoint *
WKBReaderT<InStream>::readMultiPoint()
{
	int numGeoms = dis.readInt();
	vector<Geometry *> *geoms = new vector<Geometry *>(numGeoms);

	try {
		for (int i=0; i<numGeoms; i++)
		{
			Geometry *g = readGeometry();
			if (!dynamic_cast<Point *>(g))
				throw new ParseException(BAD_GEOM_TYPE_MSG+
					" MultiPoint");
			(*geoms)[i] = g;
		}
	} catch (...) {
		for (int i=0; i<geoms->size(); i++)
			delete (*geoms)[i];
		delete geoms;
		throw;
	}
	return factory.createMultiPoint(geoms);
}

template<class InStream> MultiLineString *
WKBReaderT<InStream>::readMultiLineString()
{
	int numGeoms = dis.readInt();
	vector<Geometry *> *geoms = new vector<Geometry *>(numGeoms);

	try {
		for (int i=0; i<numGeoms; i++)
		{
			Geometry *g = readGeometry();
			if (!dynamic_cast<LineString *>(g))
				throw new ParseException(BAD_GEOM_TYPE_MSG+
					" LineString");
			(*geoms)[i] = g;
		}
	} catch (...) {
		for (int i=0; i<geoms->size(); i++)
			delete (*geoms)[i];
		delete geoms;
		throw;
	}
	return factory.createMultiLineString(geoms);
}

template<class InStream> MultiPolygon *
WKBReaderT<InStream>::readMultiPolygon()
{
	int numGeoms = dis.readInt();
	vector<Geometry *> *geoms = new vector<Geometry *>(numGeoms);

	try {
		for (int i=0; i<numGeoms; i++)
		{
			Geometry *g = readGeometry();
			if (!dynamic_cast<Polygon *>(g))
				throw new ParseException(BAD_GEOM_TYPE_MSG+
					" Polygon");
			(*geoms)[i] = g;
		}
	} catch (...) {
		for (int i=0; i<geoms->size(); i++)
			delete (*geoms)[i];
		delete geoms;
		throw;
	}
	return factory.createMultiPolygon(geoms);
}

template<class InStream> GeometryCollection *
WKBReaderT<InStream>::readGeometryCollection()
{
	int numGeoms = dis.readInt();
	vector<Geometry *> *geoms = new vector<Geometry *>(numGeoms);

	try {
		for (int i=0; i<numGeoms; i++)
			(*geoms)[i] = (readGeometry());
	} catch (...) {
		for (int i=0; i<geoms->size(); i++)
			delete (*geoms)[i];
		delete geoms;
		throw;
	}
	return factory.createGeometryCollection(geoms);
}

template<class InStream> CoordinateSequence *
WKBReaderT<InStream>::readCoordinateSequence(int size)
{
	CoordinateSequence *seq = factory.getCoordinateSequenceFactory()->create(size, inputDimension);
	int targetDim = seq->getDimension();
	if ( targetDim > inputDimension )
		targetDim = inputDimension;
	for (int i=0; i<size; i++) {
		readCoordinate();
		for (int j=0; j<targetDim; j++) {
			seq->setOrdinate(i, j, ordValues[j]);
		}
	}
	return seq;
}

template<class InStream> void
WKBReaderT<InStream>::readCoordinate()
{
	for (int i=0; i<inputDimension; ++i)
	{
		ordValues[i] = dis.readDouble();
	}
}

} // namespace geos

#endif // _WKBREADERT_H
