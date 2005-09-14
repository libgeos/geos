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

#include <geos/io.h>

namespace geos {

string WKBReader::BAD_GEOM_TYPE_MSG = "bad geometry type encountered in ";

ostream &
WKBReader::printHEX(istream &is, ostream &os)
{
	ios_base::fmtflags fl = os.flags(); // take note of output stream flags

	// Set hex,uppercase,fill and width output stream flags
	os.setf(ios::uppercase);
	os.setf(ios::hex, ios::basefield);
	os.setf(ios::hex, ios::basefield);
	os.fill('0');

	long pos = is.tellg(); // take note of input stream get pointer
	is.seekg(0, ios::beg); // rewind

	byte each=0;
	while(is.read(reinterpret_cast<char *>(&each), 1))
		os<<setw(2)<<(int)each;

	is.clear(); // clear input stream eof flag
	is.seekg(pos); // reset input stream position
	os.setf(fl);  // reset output stream status
	return os;
}


Geometry *
WKBReader::read(istream &is)
{
	dis.setInStream(&is); // will default to machine endian
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
	bool hasZ = ((typeInt & 0x80000000) != 0);
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

Point *
WKBReader::readPoint()
{
	readCoordinate();
#if DEBUG_WKB_READER
	cout<<"Coordinates: "<<ordValues[0]<<","<<ordValues[1]<<endl;
#endif
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
				(*holes)[i] = (Geometry *)readLinearRing();
		} catch (...) {
			for (unsigned int i=0; i<holes->size(); i++)
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
				throw new ParseException(BAD_GEOM_TYPE_MSG+
					" MultiPoint");
			(*geoms)[i] = g;
		}
	} catch (...) {
		for (unsigned int i=0; i<geoms->size(); i++)
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
				throw new ParseException(BAD_GEOM_TYPE_MSG+
					" LineString");
			(*geoms)[i] = g;
		}
	} catch (...) {
		for (unsigned int i=0; i<geoms->size(); i++)
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
				throw new ParseException(BAD_GEOM_TYPE_MSG+
					" Polygon");
			(*geoms)[i] = g;
		}
	} catch (...) {
		for (unsigned int i=0; i<geoms->size(); i++)
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
		for (unsigned int i=0; i<geoms->size(); i++)
			delete (*geoms)[i];
		delete geoms;
		throw;
	}
	return factory.createGeometryCollection(geoms);
}

CoordinateSequence *
WKBReader::readCoordinateSequence(int size)
{
	CoordinateSequence *seq = factory.getCoordinateSequenceFactory()->create(size, inputDimension);
	unsigned int targetDim = seq->getDimension();
	if ( targetDim > inputDimension )
		targetDim = inputDimension;
	for (int i=0; i<size; i++) {
		readCoordinate();
		for (unsigned int j=0; j<targetDim; j++) {
			seq->setOrdinate(i, j, ordValues[j]);
		}
	}
	return seq;
}

void
WKBReader::readCoordinate()
{
	for (unsigned int i=0; i<inputDimension; ++i)
	{
		ordValues[i] = dis.readDouble();
	}
}

} // namespace geos
