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

#pragma once

#include <geos/export.h>
#include <geos/io/ParseException.h>
#include <geos/io/ByteOrderValues.h>
#include <geos/util/Machine.h> // for getMachineByteOrder

#include <cstdint>
#include <iosfwd> // ostream, istream (if we remove inlines)

namespace geos {
namespace io {

/**
 * \class ByteOrderDataInStream
 *
 * \brief Allows reading an stream of primitive datatypes from an underlying
 * istream, with the representation being in either common byte ordering.
 *
 */
class GEOS_DLL ByteOrderDataInStream {

public:

    ByteOrderDataInStream()
        : ByteOrderDataInStream(nullptr, 0) {};

    ByteOrderDataInStream(const unsigned char* buff, size_t buffsz)
        : byteOrder(getMachineByteOrder())
        , buf(buff)
        , end(buff + buffsz)
        {};

    ~ByteOrderDataInStream() {};

    void setOrder(int order)
    {
        byteOrder = order;
    };

    unsigned char readByte() // throws ParseException
    {
        if(size() < 1) {
            throw  ParseException("Unexpected EOF parsing WKB");
        }
        auto ret = buf[0];
        buf++;
        return ret;
    };

    int32_t readInt()
    {
        if(size() < 4) {
            throw ParseException("Unexpected EOF parsing WKB");
        }
        auto ret =  ByteOrderValues::getInt(buf , byteOrder);
        buf += 4;
        return ret;
    };

    uint32_t readUnsigned()
    {
        if(size() < 4) {
            throw ParseException("Unexpected EOF parsing WKB");
        }
        auto ret =  ByteOrderValues::getUnsigned(buf , byteOrder);
        buf += 4;
        return ret;
    };

    int64_t readLong()
    {
        if(size() < 8) {
            throw ParseException("Unexpected EOF parsing WKB");
        }

        auto ret = ByteOrderValues::getLong(buf, byteOrder);
        buf += 8;
        return ret;
    };

    double readDouble()
    {
        if(size() < 8) {
            throw  ParseException("Unexpected EOF parsing WKB");
        }
        auto ret = ByteOrderValues::getDouble(buf, byteOrder);
        buf += 8;
        return ret;
    };

    size_t size() const
    {
        return static_cast<size_t>(end - buf);
    };


private:
    int byteOrder;
    const unsigned char* buf;
    const unsigned char* end;

};

} // namespace io
} // namespace geos
