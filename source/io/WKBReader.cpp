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


#include <geos/io.h>
#include <geos/util.h>

namespace geos {

namespace io {

WKBReader::BAD_GEOM_TYPE_MSG = "bad geometry type encountered in ";

Geometry *
WKBReader::read(InStream &is)
{
	dis.setInStream(is);
	inputDimension = 2; // handle 2d only for now

	// only allocate ordValues buffer if necessary
	if ( ordValues.size() < inputDimension )
		ordValues.resize(inputDimension);

	return readGeometry();
}

Geometry *
WKBReader::readGeometry()
{
	// determine byte order
	byte byteOrder = dis.readByte();
	// default is big endian
	if (byteOrder == WKBConstants.wkbNDR)
		dis.setOrder(ByteOrderValues.LITTLE_ENDIAN);

	int typeInt = dis.readInt();
	int geometryType = typeInt & 0xff;
	boolean hasZ = (typeInt & 0x80000000) != 0;
	if (hasZ) inputDimension = 3;

	switch (geometryType) {
		case WKBConstants.wkbPoint :
			return readPoint();
		case WKBConstants.wkbLineString :
			return readLineString();
		case WKBConstants.wkbPolygon :
			return readPolygon();
		case WKBConstants.wkbMultiPoint :
			return readMultiPoint();
		case WKBConstants.wkbMultiLineString :
			return readMultiLineString();
		case WKBConstants.wkbMultiPolygon :
			return readMultiPolygon();
		case WKBConstants.wkbGeometryCollection :
			return readGeometryCollection();
	}
	throw new ParseException("Unknown WKB type " + geometryType);
}

Point *
WKBReader::readPoint()
{
	readCoordinate();
	return factory.createPoint(Coordinate(ordValues[0], ordValues[1]));
}

LineString *
WKBReader::readLineString()
{
	int size = dis.readInt();
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLineString(pts);
}

LinearRing *
WKBReader::readLinearRing()
{
	int size = dis.readInt();
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLinearRing(pts);
}

Polygon *
WKBReader::readPolygon()
{
	int numRings = dis.readInt();
	LinearRing *shell = readLinearRing();

	vector<Geometry *>*holes=NULL;
	if ( numRings > 1 )
	{
		try {
			holes = new vector<Geometry *>(numRings-1);
			for (int i=0; i<numRings-1; i++)
				(*holes)[i]((Geometry *)readLinearRing());
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

MultiPoint *
WKBReader::readMultiPoint()
{
	int numGeoms = dis.readInt();
	vector<Geometry *> *geoms = new vector<Geometry *>(numGeoms);

	try {
		for (int i=0; i<numGeoms; i++)
		{
			Geometry *g = readGeometry();
			if (!dynamic_cast<Point *>(g))
				throw new ParserException(BAD_GEOM_TYPE_MSG+
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

MultiLineString *
WKBReader::readMultiLineString()
{
	int numGeoms = dis.readInt();
	vector<Geometry *> *geoms = new vector<Geometry *>(numGeoms);

	try {
		for (int i=0; i<numGeoms; i++)
		{
			Geometry *g = readGeometry();
			if (!dynamic_cast<LineString *>(g))
				throw new ParserException(BAD_GEOM_TYPE_MSG+
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

MultiPolygon *
WKBReader::readMultiPolygon()
{
	int numGeoms = dis.readInt();
	vector<Geometry *> *geoms = new vector<Geometry *>(numGeoms);

	try {
		for (int i=0; i<numGeoms; i++)
		{
			Geometry *g = readGeometry();
			if (!dynamic_cast<Polygon *>(g))
				throw new ParserException(BAD_GEOM_TYPE_MSG+
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

GeometryCollection *
WKBReader::readGeometryCollection()
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

CoordinateSequence *
WKBReader::readCoordinateSequence(int size)
{
	CoordinateSequence *seq = factory->getCoordinateSequenceFactory()->create(size, inputDimensions);
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

void
WKBReader::readCoordinate()
{
	for (int i=0; i<inputDimension; ++i)
	{
		ordValues[i] = dis.readDouble();
	}
}

} // namespace io

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2005/03/10 17:27:40  strk
 * ported from current JTS
 *
 **********************************************************************/
