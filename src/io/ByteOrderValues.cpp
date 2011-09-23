/**********************************************************************
 * $Id: ByteOrderValues.cpp 3253 2011-03-01 17:50:36Z mloskot $
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
 **********************************************************************
 *
 * Last port: io/ByteOrderValues.java rev. 1.3 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/io/ByteOrderValues.h>
#include <geos/platform.h>
#include <geos/util.h>

#include <cstring>
#include <cassert>

namespace geos {
namespace io { // geos.io

int
ByteOrderValues::getInt(const unsigned char *buf, int byteOrder)
{
	if ( byteOrder == ENDIAN_BIG )
	{
		return  ((int) (buf[0]&0xff) <<24) |
			((int) (buf[1]&0xff) <<16) |
			((int) (buf[2]&0xff) <<8) |
			((int) (buf[3]&0xff) );
	}
	else // ENDIAN_LITTLE
	{
		assert(byteOrder == ENDIAN_LITTLE);

		return  ((int) (buf[3]&0xff) <<24) |
			((int) (buf[2]&0xff) <<16) |
			((int) (buf[1]&0xff) <<8) |
			((int) (buf[0]&0xff) );
	}
}

void
ByteOrderValues::putInt(int intValue, unsigned char *buf, int byteOrder)
{
	if ( byteOrder == ENDIAN_BIG )
	{
		buf[0] = (unsigned char)(intValue >> 24);
		buf[1] = (unsigned char)(intValue >> 16);
		buf[2] = (unsigned char)(intValue >> 8);
		buf[3] = (unsigned char) intValue;
	}
	else // ENDIAN_LITTLE
	{
		assert(byteOrder == ENDIAN_LITTLE);

		buf[3] = (unsigned char)(intValue >> 24);
		buf[2] = (unsigned char)(intValue >> 16);
		buf[1] = (unsigned char)(intValue >> 8);
		buf[0] = (unsigned char) intValue;
	}
}

int64
ByteOrderValues::getLong(const unsigned char *buf, int byteOrder)
{
	if ( byteOrder == ENDIAN_BIG )
	{
		return
			(int64) (buf[0]) << 56
			| (int64) (buf[1] & 0xff) << 48
			| (int64) (buf[2] & 0xff) << 40
			| (int64) (buf[3] & 0xff) << 32
			| (int64) (buf[4] & 0xff) << 24
			| (int64) (buf[5] & 0xff) << 16
			| (int64) (buf[6] & 0xff) <<  8
			| (int64) (buf[7] & 0xff);
	}
	else // ENDIAN_LITTLE
	{
		assert(byteOrder == ENDIAN_LITTLE);

		return
			(int64) (buf[7]) << 56
			| (int64) (buf[6] & 0xff) << 48
			| (int64) (buf[5] & 0xff) << 40
			| (int64) (buf[4] & 0xff) << 32
			| (int64) (buf[3] & 0xff) << 24
			| (int64) (buf[2] & 0xff) << 16
			| (int64) (buf[1] & 0xff) <<  8
			| (int64) (buf[0] & 0xff);
	}
}

void
ByteOrderValues::putLong(int64 longValue, unsigned char *buf, int byteOrder)
{
	if ( byteOrder == ENDIAN_BIG )
	{
		buf[0] = (unsigned char)(longValue >> 56);
		buf[1] = (unsigned char)(longValue >> 48);
		buf[2] = (unsigned char)(longValue >> 40);
		buf[3] = (unsigned char)(longValue >> 32);
		buf[4] = (unsigned char)(longValue >> 24);
		buf[5] = (unsigned char)(longValue >> 16);
		buf[6] = (unsigned char)(longValue >> 8);
		buf[7] = (unsigned char) longValue;
	}
	else // ENDIAN_LITTLE
	{
		assert(byteOrder == ENDIAN_LITTLE);

		buf[0] = (unsigned char) longValue;
		buf[1] = (unsigned char)(longValue >> 8);
		buf[2] = (unsigned char)(longValue >> 16);
		buf[3] = (unsigned char)(longValue >> 24);
		buf[4] = (unsigned char)(longValue >> 32);
		buf[5] = (unsigned char)(longValue >> 40);
		buf[6] = (unsigned char)(longValue >> 48);
		buf[7] = (unsigned char)(longValue >> 56);
	}
}

double
ByteOrderValues::getDouble(const unsigned char *buf, int byteOrder)
{
	int64 longValue = getLong(buf, byteOrder);
	double ret;
    std::memcpy(&ret, &longValue, sizeof(double));
	return ret;
}

void
ByteOrderValues::putDouble(double doubleValue, unsigned char *buf, int byteOrder)
{
	int64 longValue;
    std::memcpy(&longValue, &doubleValue, sizeof(double));
#if DEBUG_BYTEORDER_VALUES
	cout<<"ByteOrderValues::putDouble("<<doubleValue<<
		", order:"<<byteOrder
		<<") = "<<hex;
	for (int i=0; i<8; i++)
		cout<<"["<<(int)buf[i]<<"]";
	cout<<dec<<endl;
#endif
	putLong(longValue, buf, byteOrder);
}

} // namespace geos.io
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.9  2006/05/23 12:33:41  strk
 * * source/io/ByteOrderValues.cpp: fixed bug in putLong() also affecting putDouble()
 *
 * Revision 1.8  2006/05/23 09:24:35  strk
 * * source/io/ByteOrderValues.cpp: changed ENDIAN_BIG and ENDIAN_LITTLE values to match WKBConstants::XDR and WKBConstants::NDR respectively.
 * * source/headers/geos/io/WKBConstants.h: added comments about meaning of XDR/NDR.
 *
 * Revision 1.7  2006/03/20 18:18:15  strk
 * io.h header split
 *
 * Revision 1.6  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.5  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.4  2006/01/21 16:08:30  strk
 * Fixed integer conversion bug (ported from JTS-1.7).
 * Added last port information.
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
 * Revision 1.1  2005/04/20 17:22:47  strk
 * Added initial implementation of WKBReaderT and ByteOrderDataInStreamT
 * class templates and ByteOrderValues class.
 * Work is unfinished as WKBReader requires new interface of CoordinateSequence
 * taking higher dimensions into account.
 *
 **********************************************************************/
