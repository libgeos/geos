/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005 Refractions Research Inc.
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
#include <geos/platform.h>
#include <geos/ByteOrderDataInStreamT.h>

#define DEBUG_WKB_READER 0

namespace geos {

/*
 * \class WKBReaderT io.h geos.h
 * \brief WKB parser class; see also WKBWriter.
 *
 * Reads a Geometry from a byte stream in Well-Known Binary format.
 * Supports use of an T, which allows easy use
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
template <class T>
class WKBReaderT {

public:

	WKBReaderT() {};
	WKBReaderT(const GeometryFactory &f): factory(f) {};

	/**
	 * Reads a Geometry from an T.
	 *
	 * @param is the stream to read from 
	 * @return the Geometry read
	 * @throws IOException
	 * @throws ParseException
	 */
	Geometry *read(T &is);
		// throws IOException, ParseException
 
private:

	static string BAD_GEOM_TYPE_MSG;

	const GeometryFactory factory;

	// for now support the WKB standard only - may be generalized later
	int inputDimension;

	ByteOrderDataInStreamT<T> dis;

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


};
template<class T> string WKBReaderT<T>::BAD_GEOM_TYPE_MSG = "bad geometry type encountered in ";

template<class T> Geometry *
WKBReaderT<T>::read(T &is)
{
	dis.setInStream(&is); // will default to machine endian
	inputDimension = 2; // handle 2d only for now

	// only allocate ordValues buffer if necessary
	if ( ordValues.size() < inputDimension )
		ordValues.resize(inputDimension);

	return readGeometry();
}

template<class T> Geometry *
WKBReaderT<T>::readGeometry()
{
	// determine byte order
	byte byteOrder = dis.readByte();

#if DEBUG_WKB_READER
	cout<<"WKB byteOrder: "<<(int)byteOrder<<endl;
#endif

	// default is machine endian
	if (byteOrder == WKBConstants::wkbNDR)
		dis.setOrder(ByteOrderValues::ENDIAN_LITTLE);

	int typeInt = dis.readInt();
#if DEBUG_WKB_READER
	cout<<"WKB type: "<<typeInt<<endl;
#endif

	int geometryType = typeInt & 0xff;
	bool hasZ = (typeInt & 0x80000000) != 0;
	if (hasZ) inputDimension = 3;

#if DEBUG_WKB_READER
	cout<<"WKB dimensions: "<<inputDimension<<endl;
#endif

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

template<class T> Point *
WKBReaderT<T>::readPoint()
{
	readCoordinate();
#if DEBUG_WKB_READER
	cout<<"Coordinates: "<<ordValues[0]<<","<<ordValues[1]<<endl;
#endif
	return factory.createPoint(Coordinate(ordValues[0], ordValues[1]));
}

template<class T> LineString *
WKBReaderT<T>::readLineString()
{
	int size = dis.readInt();
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLineString(pts);
}

template<class T> LinearRing *
WKBReaderT<T>::readLinearRing()
{
	int size = dis.readInt();
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLinearRing(pts);
}

template<class T> Polygon *
WKBReaderT<T>::readPolygon()
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

template<class T> MultiPoint *
WKBReaderT<T>::readMultiPoint()
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

template<class T> MultiLineString *
WKBReaderT<T>::readMultiLineString()
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

template<class T> MultiPolygon *
WKBReaderT<T>::readMultiPolygon()
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

template<class T> GeometryCollection *
WKBReaderT<T>::readGeometryCollection()
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

template<class T> CoordinateSequence *
WKBReaderT<T>::readCoordinateSequence(int size)
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

template<class T> void
WKBReaderT<T>::readCoordinate()
{
	for (int i=0; i<inputDimension; ++i)
	{
		ordValues[i] = dis.readDouble();
	}
}

// biostringstream-based WKB reader
typedef WKBReaderT<biostringstream> WKBReader;


} // namespace geos

#endif // _WKBREADERT_H
