/**********************************************************************
 * $Id: GeometryComponentFilter.cpp 2757 2009-12-01 15:39:41Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <cassert>

#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/Geometry.h>
#include <geos/util.h>

namespace geos {
namespace geom { // geos::geom

void GeometryComponentFilter::filter_rw(Geometry *geom)
{
	::geos::ignore_unused_variable_warning(geom);
	assert(0);
}

void GeometryComponentFilter::filter_ro(const Geometry *geom)
{
	::geos::ignore_unused_variable_warning(geom);
	assert(0);
}


} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.12  2006/04/13 14:25:17  strk
 * TopologyPreservingSimplifier initial port
 *
 * Revision 1.11  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
 **********************************************************************/

