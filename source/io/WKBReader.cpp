/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/io/WKBReader.h>
#include <geos/io/WKBConstants.h>
#include <geos/io/ByteOrderValues.h>
#include <geos/io/ParseException.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace io { // geos.io

string WKBReader::BAD_GEOM_TYPE_MSG = "bad geometry type encountered in ";

ostream &
WKBReader::printHEX(istream &is, ostream &os)
{
	static const char hex[] = "0123456789ABCDEF";

	long pos = is.tellg(); // take note of input stream get pointer
	is.seekg(0, ios::beg); // rewind input stream

	char each=0;
	while(is.read(&each, 1))
	{
		const unsigned char c=each;
		int low = (c & 0x0F);
		int high = (c >> 4);
		os << hex[high] << hex[low];
	}

	is.clear(); // clear input stream eof flag
	is.seekg(pos); // reset input stream position

	return os;
}

Geometry *
WKBReader::readHEX(istream &is)
{
	// setup input/output stream
	stringstream os(ios_base::binary|ios_base::in|ios_base::out);

	unsigned char high, low, result_high, result_low, value;

	while(!is.eof())//readsome(&str[0], 2))
	{
		// get the high part of the byte
		is >> high;
		// geth the low part of the byte
		is >> low;

		switch (high)
		{
			case '0' :
				result_high = 0;
				break;
			case '1' :
				result_high = 1;
				break;
			case '2' :
				result_high = 2;
				break;
			case '3' :
				result_high = 3;
				break;
			case '4' :
				result_high = 4;
				break;
			case '5' :
				result_high = 5;
				break;
			case '6' :
				result_high = 6;
				break;
			case '7' :
				result_high = 7;
				break;
			case '8' :
				result_high = 8;
				break;
			case '9' :
				result_high = 9;
				break;
			case 'A' :
				result_high = 10;
				break;
			case 'B' :
				result_high = 11;
				break;
			case 'C' :
				result_high = 12;
				break;
			case 'D' :
				result_high = 13;
				break;
			case 'E' :
				result_high = 14;
				break;
			case 'F' :
				result_high = 15;
				break;
			default:
				throw  ParseException("Invalid HEX char");
		}

		switch (low)
		{
			case '0' :
				result_low = 0;
				break;
			case '1' :
				result_low = 1;
				break;
			case '2' :
				result_low = 2;
				break;
			case '3' :
				result_low = 3;
				break;
			case '4' :
				result_low = 4;
				break;
			case '5' :
				result_low = 5;
				break;
			case '6' :
				result_low = 6;
				break;
			case '7' :
				result_low = 7;
				break;
			case '8' :
				result_low = 8;
				break;
			case '9' :
				result_low = 9;
				break;
			case 'A' :
				result_low = 10;
				break;
			case 'B' :
				result_low = 11;
				break;
			case 'C' :
				result_low = 12;
				break;
			case 'D' :
				result_low = 13;
				break;
			case 'E' :
				result_low = 14;
				break;
			case 'F' :
				result_low = 15;
				break;
			default:
				throw  ParseException("Invalid HEX char");
		}

		value = (result_high<<4) + result_low;

#if DEBUG_HEX_READER
	cout<<"HEX "<<high<<low<<" -> DEC "<<(int)value<<endl;
#endif
		// write the value to the output stream
		os << value;
	}

	// now call read to convert the geometry
	return this->read(os);
}

Geometry *
WKBReader::read(istream &is)
{
	dis.setInStream(&is); // will default to machine endian
	return readGeometry();
}

Geometry *
WKBReader::readGeometry()
{
	// determine byte order
	unsigned char byteOrder = dis.readByte();

#if DEBUG_WKB_READER
	cout<<"WKB byteOrder: "<<(int)byteOrder<<endl;
#endif

	// default is machine endian
	if (byteOrder == WKBConstants::wkbNDR)
		dis.setOrder(ByteOrderValues::ENDIAN_LITTLE);

	int typeInt = dis.readInt();
	int geometryType = typeInt & 0xff;

#if DEBUG_WKB_READER
	cout<<"WKB geometryType: "<<geometryType<<endl;
#endif

	bool hasZ = ((typeInt & 0x80000000) != 0);
	if (hasZ) inputDimension = 3;
	else inputDimension = 2; // doesn't handle M currently

#if DEBUG_WKB_READER
	cout<<"WKB hasZ: "<<hasZ<<endl;
#endif

#if DEBUG_WKB_READER
	cout<<"WKB dimensions: "<<inputDimension<<endl;
#endif

	bool hasSRID = ((typeInt & 0x20000000) != 0);

#if DEBUG_WKB_READER
	cout<<"WKB hasSRID: "<<hasZ<<endl;
#endif

	int SRID = -1;
	if (hasSRID) SRID = dis.readInt(); // read SRID


	// allocate space for ordValues 
	if ( ordValues.size() < inputDimension )
		ordValues.resize(inputDimension);

	Geometry *result;

	switch (geometryType) {
		case WKBConstants::wkbPoint :
			result = readPoint();
			break;
		case WKBConstants::wkbLineString :
			result = readLineString();
			break;
		case WKBConstants::wkbPolygon :
			result = readPolygon();
			break;
		case WKBConstants::wkbMultiPoint :
			result = readMultiPoint();
			break;
		case WKBConstants::wkbMultiLineString :
			result = readMultiLineString();
			break;
		case WKBConstants::wkbMultiPolygon :
			result = readMultiPolygon();
			break;
		case WKBConstants::wkbGeometryCollection :
			result = readGeometryCollection();
			break;
		default:
			throw  ParseException("Unknown WKB type " +
					geometryType);
	}

	result->setSRID(SRID);
	return result;
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
#if DEBUG_WKB_READER
	cout<<"WKB npoints: "<<size<<endl;
#endif
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLineString(pts);
}

LinearRing *
WKBReader::readLinearRing()
{
	int size = dis.readInt();
#if DEBUG_WKB_READER
	cout<<"WKB npoints: "<<size<<endl;
#endif
	CoordinateSequence *pts = readCoordinateSequence(size);
	return factory.createLinearRing(pts);
}

Polygon *
WKBReader::readPolygon()
{
	int numRings = dis.readInt();

#if DEBUG_WKB_READER
	cout<<"WKB numRings: "<<numRings<<endl;
#endif

        LinearRing *shell = NULL;
        if( numRings > 0 )
            shell = readLinearRing();

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
				throw  ParseException(BAD_GEOM_TYPE_MSG+
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
				throw  ParseException(BAD_GEOM_TYPE_MSG+
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
				throw  ParseException(BAD_GEOM_TYPE_MSG+
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
	static const PrecisionModel &pm = *factory.getPrecisionModel();
	for (unsigned int i=0; i<inputDimension; ++i)
	{
		if ( i <= 1 ) ordValues[i] = pm.makePrecise(dis.readDouble());
		else ordValues[i] = dis.readDouble();
	}
#if DEBUG_WKB_READER
	cout<<"WKB coordinate: "<<ordValues[0]<<","<<ordValues[1]<<endl;
#endif
}

} // namespace geos.io
} // namespace geos
