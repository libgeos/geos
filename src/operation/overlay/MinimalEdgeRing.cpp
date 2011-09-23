/**********************************************************************
 * $Id: MinimalEdgeRing.cpp 2564 2009-06-08 16:09:51Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/overlay/MinimalEdgeRing.java rev. 1.13 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/operation/overlay/MinimalEdgeRing.h>
#include <geos/geomgraph/EdgeRing.h>

#ifndef GEOS_INLINE
#include <geos/operation/overlay/MinimalEdgeRing.inl>
#endif

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

MinimalEdgeRing::MinimalEdgeRing(geomgraph::DirectedEdge *start,
		const geom::GeometryFactory *geometryFactory)
	:
	geomgraph::EdgeRing(start, geometryFactory)
{
	computePoints(start);
	computeRing();
#if GEOS_DEBUG
	std::cerr << "MinimalEdgeRing[" << this << "] ctor" << std::endl;
#endif
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.14  2006/03/27 16:02:34  strk
 * Added INL file for MinimalEdgeRing, added many debugging blocks,
 * fixed memory leak in ConnectedInteriorTester (bug #59)
 *
 * Revision 1.13  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.12  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 **********************************************************************/

