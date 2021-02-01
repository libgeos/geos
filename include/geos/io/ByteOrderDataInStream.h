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

#ifndef GEOS_IO_BYTEORDERDATAINSTREAM_H
#define GEOS_IO_BYTEORDERDATAINSTREAM_H

#include <geos/export.h>
#include <cstdint>

//#include <geos/io/ParseException.h>
//#include <geos/io/ByteOrderValues.h>
#include <geos/inline.h>

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

    ByteOrderDataInStream(const unsigned char* buff = nullptr, size_t buff_sz = 0);

    ~ByteOrderDataInStream();

    void setOrder(int order);

    unsigned char readByte(); // throws ParseException

    int32_t readInt(); // throws ParseException

    uint32_t readUnsigned(); // throws ParseException

    int64_t readLong(); // throws ParseException

    double readDouble(); // throws ParseException

    size_t size() const;

private:
    int byteOrder;
    const unsigned char* buf;
    const unsigned char* end;
};

} // namespace io
} // namespace geos

#ifdef GEOS_INLINE
#include <geos/io/ByteOrderDataInStream.inl>
#endif

#endif // #ifndef GEOS_IO_BYTEORDERDATAINSTREAM_H
