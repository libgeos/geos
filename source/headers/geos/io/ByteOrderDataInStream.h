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

#ifndef GEOS_IO_BYTEORDERDATAINSTREAM_H
#define GEOS_IO_BYTEORDERDATAINSTREAM_H

#include <geos/platform.h>
#include <geos/io/ParseException.h>
#include <geos/io/ByteOrderValues.h>
#include <iostream> // ostream, istream (due to inlines)
//#include <iosfwd> // ostream, istream (if we remove inlines)

namespace geos {
namespace io {

/*
 * \class ByteOrderDataInStream io.h geos.h
 * 
 * Allows reading an stream of primitive datatypes from an underlying
 * istream, with the representation being in either common byte ordering.
 *
 */
class ByteOrderDataInStream {

public:

	ByteOrderDataInStream(std::istream *s=NULL)
		: byteOrder(getMachineByteOrder()), stream(s)
	{}
	~ByteOrderDataInStream() {}

	/**
	 * Allows a single ByteOrderDataInStreamT to be reused
	 * on multiple istream.
	 */
	void setInStream(std::istream *s) { stream=s; }
	void setOrder(int order) { byteOrder=order; }

	unsigned char readByte() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 1);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return buf[0];
	}

	int readInt() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 4);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return ByteOrderValues::getInt(buf, byteOrder);
	}

	long readLong() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 8);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return ByteOrderValues::getLong(buf, byteOrder);
	}

	double readDouble() // throws ParseException
	{
		stream->read(reinterpret_cast<char *>(buf), 8);
		if ( stream->eof() )
			throw  ParseException("Unexpected EOF parsing WKB");
		return ByteOrderValues::getDouble(buf, byteOrder);
	}

private:
	int byteOrder;
	std::istream *stream;

	// buffers to hold primitive datatypes
	unsigned char buf[8];

};

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_BYTEORDERDATAINSTREAM_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 18:18:14  strk
 * io.h header split
 *
 **********************************************************************/
