/**********************************************************************
 * $Id: WKBConstants.h 2579 2009-06-15 14:03:52Z strk $
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
 **********************************************************************
 *
 * Last port: io/WKBConstants.java rev. 1.1 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_IO_WKBCONSTANTS_H
#define GEOS_IO_WKBCONSTANTS_H

namespace geos {
namespace io {

/// Constant values used by the WKB format
namespace WKBConstants {

	/// Big Endian
	const int wkbXDR = 0;

	/// Little Endian 
	const int wkbNDR = 1;

	const int wkbPoint = 1;
	const int wkbLineString = 2;
	const int wkbPolygon = 3;
	const int wkbMultiPoint = 4;
	const int wkbMultiLineString = 5;
	const int wkbMultiPolygon = 6;
	const int wkbGeometryCollection = 7;
}

} // namespace geos::io
} // namespace geos

#endif // #ifndef GEOS_IO_WKBCONSTANTS_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/05/23 09:24:37  strk
 * * source/io/ByteOrderValues.cpp: changed ENDIAN_BIG and ENDIAN_LITTLE values to match WKBConstants::XDR and WKBConstants::NDR respectively.
 * * source/headers/geos/io/WKBConstants.h: added comments about meaning of XDR/NDR.
 *
 * Revision 1.1  2006/03/20 18:18:14  strk
 * io.h header split
 *
 **********************************************************************/
