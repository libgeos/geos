/**********************************************************************
 * $Id$
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
 **********************************************************************/

#ifndef GEOS_IO_WKTREADER_INL
#define GEOS_IO_WKTREADER_INL

namespace geos {
	namespace io {

INLINE WKTReader::WKTReader(const GeometryFactory *gf)
{
#if GEOS_DEBUG
    std::cout << "\nGEOS_DEBUG: WKTReader::WKTReader(const GeometryFactory *gf)\n";
#endif

	geometryFactory=gf;
	precisionModel=gf->getPrecisionModel();
}

INLINE WKTReader::~WKTReader()
{
#if GEOS_DEBUG
    std::cout << "\nGEOS_DEBUG: WKTReader::~WKTReader()\n";
#endif
}

	} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_WKTREADER_INL

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 18:18:15  strk
 * io.h header split
 *
 **********************************************************************/
