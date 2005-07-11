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

#ifndef WKBWRITERT_H
#define WKBWRITERT_H 1

#include <geos/geom.h>
#include <geos/io.h>
#include <geos/platform.h>
#include <geos/ByteOrderDataInStreamT.h>

namespace geos {


/**
 *
 * \class WKBWriterT io.h geos.h
 *
 * \brief Writes a Geometry into Well-Known Binary format.
 *
 * This is a template class.
 * Supports use of a template arg, which allows easy use
 * with arbitary byte stream sinks. The template argument
 * must support the ``write(what, howmany)'' interface, what
 * being a byte array.
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
 * @see WKBReaderT
 * @see WKBWriter
 */
template <class T>
class WKBWriterT {

public:

	WKBWriterT(int dims=2, int bo=getMachineByteOrder()):
		outputDimension(dims), byteOrder(bo), outStream(NULL) {};

	/**
	 * \brief Write a Geometry to a T.
	 *
	 * @param g the geometry to write
	 * @param os the output stream
	 * @throws IOException
	 */
	void write(const Geometry &g, T &os);
		// throws IOException, ParseException
 

private:

	int outputDimension;

	int byteOrder;

	T *outStream;

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

template<class T> void
WKBWriterT<T>::write(const Geometry &g, T &os) 
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
		case GEOS_MULTILINESTRING:
		case GEOS_MULTIPOLYGON:
		case GEOS_GEOMETRYCOLLECTION:
			return writeGeometryCollection(
				(GeometryCollection &)g,
				WKBConstants::wkbGeometryCollection);
		default:
			Assert::shouldNeverReachHere("Unknown Geometry type");
	}
}

template<class T> void
WKBWriterT<T>::writePoint(const Point &g) 
{
	if (g.isEmpty()) throw new
IllegalArgumentException("Empty Points cannot be represented in WKB");

	writeByteOrder();
#if DEBUG_WKB_WRITER
	cout<<"ByteOrder: "<<*outStream<<endl;
#endif
	writeGeometryType(WKBConstants::wkbPoint);
#if DEBUG_WKB_WRITER
	cout<<"GeometryType: "<<*outStream<<endl;
#endif
	writeCoordinateSequence(*(g.getCoordinatesRO()), false);
#if DEBUG_WKB_WRITER
	cout<<"CoordinateSequence: "<<*outStream<<endl;
#endif
}

template<class T> void
WKBWriterT<T>::writeLineString(const LineString &g) 
{
	writeByteOrder();
	writeGeometryType(WKBConstants::wkbLineString);
	writeCoordinateSequence(*(g.getCoordinatesRO()), true);
}

template<class T> void
WKBWriterT<T>::writePolygon(const Polygon &g) 
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

template<class T> void
WKBWriterT<T>::writeGeometryCollection(const GeometryCollection &g,
	int wkbtype) 
{
	writeByteOrder();
	writeGeometryType(wkbtype);
	int ngeoms = g.getNumGeometries();
	writeInt(ngeoms);
	for (int i=0; i<ngeoms; i++)
		write(*(g.getGeometryN(i)), *outStream);
}

template<class T> void
WKBWriterT<T>::writeByteOrder() 
{
	outStream->write(reinterpret_cast<byte*>(&byteOrder), 1);
}

template<class T> void
WKBWriterT<T>::writeGeometryType(int typeId) 
{
	writeInt(typeId);
}

template<class T> void
WKBWriterT<T>::writeInt(int val) 
{
	outStream->write(reinterpret_cast<byte *>(&val), 4);
}

template<class T> void
WKBWriterT<T>::writeCoordinateSequence(const CoordinateSequence &cs,
	bool sized) 
{
	int size = cs.getSize();
	bool is3d=false;
	if ( cs.getDimension() > 2 && outputDimension > 2) is3d = true;

	if (sized) writeInt(size);
	for (int i=0; i<size; i++) writeCoordinate(cs, i, is3d);
}

template<class T> void
WKBWriterT<T>::writeCoordinate(const CoordinateSequence &cs, int idx,
	bool is3d) 
{
#if DEBUG_WKB_WRITER
	cout<<"writeCoordinate: X:"<<cs.getX(idx)<<" Y:"<<cs.getY(idx)<<endl;
#endif
	ByteOrderValues::putDouble(cs.getX(idx), buf, byteOrder);
	outStream->write(buf, 8);
	ByteOrderValues::putDouble(cs.getY(idx), buf, byteOrder);
	outStream->write(buf, 8);
	if ( is3d )
	{
		ByteOrderValues::putDouble(
			cs.getOrdinate(idx, CoordinateSequence::X),
			buf, byteOrder);
		outStream->write(buf, 8);
	}
}

/**
 *
 * \brief The default WKBWriterT template class instance.
 * 
 * Uses biostringstream as template parameter.
 *
 * @see WKBReader
 */
typedef WKBWriterT<biostringstream> WKBWriter;

} // namespace geos

#endif // WKBWRITERT_H

/**********************************************************************
 * $Log$
 * Revision 1.5  2005/07/11 16:21:42  strk
 * Removed '_' prefix from header guards
 *
 * Revision 1.4  2005/05/09 21:17:57  strk
 * Second argument to ::write made mandatory.
 *
 * Revision 1.3  2005/04/29 17:40:36  strk
 * Updated Doxygen documentation and some Copyright headers.
 *
 * Revision 1.2  2005/04/29 15:34:21  strk
 * Typedef'ed biostringstream, preferred parameter for
 * WKB parser templates.
 * Added << operator for biostringstream.
 * Typedef'ed WKBWriter and WKBReader to be parametrized by
 * biostringstream.
 * Added WKBtest in doc/example.cpp
 *
 * Revision 1.1  2005/04/29 11:52:40  strk
 * Added new JTS interfaces for CoordinateSequence and factories,
 * removed example implementations to reduce maintainance costs.
 * Added first implementation of WKBWriter, made ByteOrderDataInStream
 * a template class.
 *
 **********************************************************************/
