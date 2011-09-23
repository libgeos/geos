/**********************************************************************
 * $Id: WKTReader.inl 2579 2009-06-15 14:03:52Z strk $
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
 * Last port: io/WKTReader.java rev. 1.1 (JTS-1.7)
 *
 **********************************************************************/

#ifndef GEOS_IO_WKTREADER_INL
#define GEOS_IO_WKTREADER_INL

#include <geos/io/WKTReader.h>
#include <geos/geom/GeometryFactory.h>

#if GEOS_DEBUG
# include <iostream>
#endif

namespace geos {
namespace io {

INLINE
WKTReader::WKTReader(const geom::GeometryFactory *gf)
	:
	geometryFactory(gf),
	precisionModel(gf->getPrecisionModel())
{
#if GEOS_DEBUG
    std::cerr << "\nGEOS_DEBUG: WKTReader::WKTReader(const GeometryFactory *gf)\n";
#endif
}

INLINE
WKTReader::WKTReader()
	:
	geometryFactory(geom::GeometryFactory::getDefaultInstance()),
	precisionModel(geometryFactory->getPrecisionModel())
{
#if GEOS_DEBUG
    std::cerr << "\nGEOS_DEBUG: WKTReader::WKTReader()\n";
#endif
}

INLINE
WKTReader::~WKTReader()
{
#if GEOS_DEBUG
    std::cerr << "\nGEOS_DEBUG: WKTReader::~WKTReader()\n";
#endif
}

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_WKTREADER_INL

/**********************************************************************
 * $Log$
 * Revision 1.5  2006/06/13 22:50:17  strk
 * * source/headers/geos/io/WKTReader.inl: added missing include for GEOS_DEBUG set case, use stderr for debugging output.
 *
 * Revision 1.4  2006/04/10 13:40:14  strk
 * Added default ctor for WKTReader (using GeometryFactory's default instance)
 *
 * Revision 1.3  2006/04/10 12:05:35  strk
 * Added inline-replicator implementation files to make sure
 * functions in .inl files are still available out-of-line.
 * A side effect is this should fix MingW build.
 *
 * Revision 1.2  2006/03/22 16:58:35  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 * Revision 1.1  2006/03/20 18:18:15  strk
 * io.h header split
 *
 **********************************************************************/
