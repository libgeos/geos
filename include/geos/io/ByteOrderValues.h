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
 * Last port: io/ByteOrderValues.java rev. 1.3 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_IO_BYTEORDERVALUES_H
#define GEOS_IO_BYTEORDERVALUES_H

#include <geos/export.h>
#include <cstdint>

namespace geos {
namespace io {

/**
 * \class ByteOrderValues
 *
 * \brief Methods to read and write primitive datatypes from/to byte
 * sequences, allowing the byte order to be specified.
 *
 * Similar to the standard Java <code>ByteBuffer</code> class.
 */
class GEOS_DLL ByteOrderValues {

public:

    enum EndianType {
        ENDIAN_BIG = 0,
        ENDIAN_LITTLE = 1
    };

    static int32_t getInt(const unsigned char* buf, int byteOrder);
    static void putInt(int32_t intValue, unsigned char* buf, int byteOrder);

    static uint32_t getUnsigned(const unsigned char* buf, int byteOrder);
    static void putUnsigned(uint32_t intValue, unsigned char* buf, int byteOrder);

    static int64_t getLong(const unsigned char* buf, int byteOrder);
    static void putLong(int64_t longValue, unsigned char* buf, int byteOrder);

    static double getDouble(const unsigned char* buf, int byteOrder);
    static void putDouble(double doubleValue, unsigned char* buf, int byteOrder);

};

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_BYTEORDERVALUES_H
