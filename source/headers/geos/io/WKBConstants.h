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

#ifndef GEOS_IO_WKBCONSTANTS_H
#define GEOS_IO_WKBCONSTANTS_H

namespace geos {
namespace io {

/// Constant values used by the WKB format
namespace WKBConstants {
	const int wkbXDR = 0;
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
 * Revision 1.1  2006/03/20 18:18:14  strk
 * io.h header split
 *
 **********************************************************************/
