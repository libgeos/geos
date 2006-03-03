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
#include <ostream>

#undef DEBUG_WKB_WRITER 

using namespace std;

namespace geos {

WKBWriter::WKBWriter(int dims, int bo):
		outputDimension(dims), byteOrder(bo), outStream(NULL)
{
	if ( dims < 2 || dims > 3 )
		throw  IllegalArgumentException("WKB output dimension must be 2 or 3");
}

void
WKBWriter::writeHEX(const Geometry &g, ostream &os) 
{
  // setup input/output stream
  stringstream stream;
  
  // write the geometry in wkb format
  this->write(g, stream);

  // convert to HEX
  WKBReader::printHEX(stream, os);
}

void
WKBWriter::write(const Geometry &g, ostream &os) 
{
	outStream = &os;

	switch (g.getGeometryTypeId()) {
		case GEOS_POINT:
			return writePoint((Point &)g);
		case GEOS_LINESTRING:
		case GEOS_LINEARRING:
			return writeLineString((LineString &)g);
		case GEOS_POLYGON:
			return writePolygon((Polygon &)g);
		case GEOS_MULTIPOINT:
			return writeGeometryCollection(
				(GeometryCollection &)g,
				WKBConstants::wkbMultiPoint);
		case GEOS_MULTILINESTRING:
			return writeGeometryCollection(
				(GeometryCollection &)g,
				WKBConstants::wkbMultiLineString);
		case GEOS_MULTIPOLYGON:
			return writeGeometryCollection(
				(GeometryCollection &)g,
				WKBConstants::wkbMultiPolygon);
		case GEOS_GEOMETRYCOLLECTION:
			return writeGeometryCollection(
				(GeometryCollection &)g,
				WKBConstants::wkbGeometryCollection);
		default:
			Assert::shouldNeverReachHere("Unknown Geometry type");
	}
}

void
WKBWriter::writePoint(const Point &g) 
{
	if (g.isEmpty()) throw 
IllegalArgumentException("Empty Points cannot be represented in WKB");

	writeByteOrder();
	writeGeometryType(WKBConstants::wkbPoint);
	writeCoordinateSequence(*(g.getCoordinatesRO()), false);
}

void
WKBWriter::writeLineString(const LineString &g) 
{
	writeByteOrder();
	writeGeometryType(WKBConstants::wkbLineString);
	writeCoordinateSequence(*(g.getCoordinatesRO()), true);
}

void
WKBWriter::writePolygon(const Polygon &g) 
{
	writeByteOrder();
	writeGeometryType(WKBConstants::wkbPolygon);
	int nholes = g.getNumInteriorRing();
	writeInt(nholes+1);
	writeCoordinateSequence(*(g.getExteriorRing()->getCoordinatesRO()),
		true);
	for (int i=0; i<nholes; i++)
		writeCoordinateSequence(
			*(g.getInteriorRingN(i)->getCoordinatesRO()),
			true);
}

void
WKBWriter::writeGeometryCollection(const GeometryCollection &g,
	int wkbtype) 
{
	writeByteOrder();
	writeGeometryType(wkbtype);
	int ngeoms = g.getNumGeometries();
	writeInt(ngeoms);
	for (int i=0; i<ngeoms; i++)
		write(*(g.getGeometryN(i)), *outStream);
}

void
WKBWriter::writeByteOrder() 
{
	outStream->write(reinterpret_cast<char*>(&byteOrder), 1);
}

void
WKBWriter::writeGeometryType(int typeId) 
{
	int flag3D = (outputDimension == 3) ? 0x80000000 : 0;
        int typeInt = typeId | flag3D;
	//writeInt(typeId);
	writeInt(typeInt);
}

void
WKBWriter::writeInt(int val) 
{
	outStream->write(reinterpret_cast<char *>(&val), 4);
}

void
WKBWriter::writeCoordinateSequence(const CoordinateSequence &cs,
	bool sized) 
{
	int size = cs.getSize();
	bool is3d=false;
	if ( cs.getDimension() > 2 && outputDimension > 2) is3d = true;

	if (sized) writeInt(size);
	for (int i=0; i<size; i++) writeCoordinate(cs, i, is3d);
}

void
WKBWriter::writeCoordinate(const CoordinateSequence &cs, int idx,
	bool is3d) 
{
#if DEBUG_WKB_WRITER
	cout<<"writeCoordinate: X:"<<cs.getX(idx)<<" Y:"<<cs.getY(idx)<<endl;
#endif
	ByteOrderValues::putDouble(cs.getX(idx), buf, byteOrder);
	outStream->write(reinterpret_cast<char *>(buf), 8);
	ByteOrderValues::putDouble(cs.getY(idx), buf, byteOrder);
	outStream->write(reinterpret_cast<char *>(buf), 8);
	if ( is3d )
	{
		ByteOrderValues::putDouble(
			cs.getOrdinate(idx, CoordinateSequence::X),
			buf, byteOrder);
		outStream->write(reinterpret_cast<char *>(buf), 8);
	}
}


} // namespace geos

