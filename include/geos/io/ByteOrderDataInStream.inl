/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: io/ByteOrderDataInStream.java rev. 1.1 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_IO_BYTEORDERDATAINSTREAM_INL
#define GEOS_IO_BYTEORDERDATAINSTREAM_INL

#include <geos/io/ParseException.h>
#include <geos/io/ByteOrderDataInStream.h>
#include <geos/io/ByteOrderValues.h>
#include <geos/util/Machine.h> // for getMachineByteOrder

#include <iostream> // ostream, istream 

namespace geos {
namespace io {

INLINE
ByteOrderDataInStream::ByteOrderDataInStream(const unsigned char* buff, size_t buffsz)
    :
    byteOrder(getMachineByteOrder()),
    buf(buff),
    end(buff + buffsz)
{
}

INLINE
ByteOrderDataInStream::~ByteOrderDataInStream()
{
}

INLINE void
ByteOrderDataInStream::setOrder(int order)
{
    byteOrder = order;
}

INLINE unsigned char
ByteOrderDataInStream::readByte() // throws ParseException
{
    if(size() < 1) {
        throw  ParseException("Unexpected EOF parsing WKB");
    }
    auto ret = buf[0];
    buf++;
    return ret;
}

INLINE int32_t
ByteOrderDataInStream::readInt()
{
    if(size() < 4) {
        throw ParseException("Unexpected EOF parsing WKB");
    }
    auto ret =  ByteOrderValues::getInt(buf , byteOrder);
    buf += 4;
    return ret;
}

INLINE uint32_t
ByteOrderDataInStream::readUnsigned()
{
    if(size() < 4) {
        throw ParseException("Unexpected EOF parsing WKB");
    }
    auto ret =  ByteOrderValues::getUnsigned(buf , byteOrder);
    buf += 4;
    return ret;
}

INLINE int64_t
ByteOrderDataInStream::readLong()
{
    if(size() < 8) {
        throw ParseException("Unexpected EOF parsing WKB");
    }

    auto ret = ByteOrderValues::getLong(buf, byteOrder);
    buf += 8;
    return ret;
}

INLINE double
ByteOrderDataInStream::readDouble()
{
    if(size() < 8) {
        throw  ParseException("Unexpected EOF parsing WKB");
    }
    auto ret = ByteOrderValues::getDouble(buf, byteOrder);
    buf += 8;
    return ret;
}

INLINE size_t
ByteOrderDataInStream::size() const
{
    return static_cast<size_t>(end - buf);
}

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_BYTEORDERDATAINSTREAM_INL
