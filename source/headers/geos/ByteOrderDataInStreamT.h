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

#ifndef BYTEORDERDATAINSTREAMT_H
#define BYTEORDERDATAINSTREAMT_H 1

#include <geos/io.h>
#include <geos/platform.h>

namespace geos {

/*
 * \class ByteOrderDataTT io.h geos.h
 * 
 * Allows reading a stream of Java primitive datatypes from an underlying
 * istream, with the representation being in either common byte ordering.
 *
 */
template<class T>
class ByteOrderDataInStreamT {

public:

	ByteOrderDataInStreamT(T *s=NULL):
		byteOrder(getMachineByteOrder()),
		stream(s) {};
	~ByteOrderDataInStreamT() {};

	/**
	 * Allows a single ByteOrderDataInStreamT to be reused
	 * on multiple istream.
	 */
	void setInStream(T *s) { stream=s; };
	void setOrder(int order) { byteOrder=order; };

	byte readByte() // throws IOException
	{
		stream->read(buf, 1);
		return buf[0];
	}

	int readInt() // throws IOException
	{
		stream->read(buf, 4);
		return ByteOrderValues::getInt(buf, byteOrder);
	}

	long readLong() // throws IOException
	{
		stream->read(buf, 8);
		return ByteOrderValues::getLong(buf, byteOrder);
	}

	double readDouble() // throws IOException
	{
		stream->read(buf, 8);
		return ByteOrderValues::getDouble(buf, byteOrder);
	}

private:
	int byteOrder;
	T *stream;

	// buffers to hold primitive datatypes
	byte buf[8];

};

} // namespace geos

#endif // BYTEORDERDATAINSTREAMT_H
