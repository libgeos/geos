/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.13  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.12  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.11  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/05/05 10:54:48  strk
 * Removed some private static heap explicit allocation, less cleanup done by
 * the unloader.
 *
 * Revision 1.9  2004/04/16 08:52:52  strk
 * Unload::Release final delete (static heap allocations should be gone now)
 *
 * Revision 1.8  2004/04/16 07:42:06  strk
 * PrecisionModel::Type made an enum instead of a Type.
 *
 * Revision 1.7  2004/04/15 15:11:38  strk
 * Commented out deletion that seems to cause segfaults
 *
 * Revision 1.6  2004/04/15 14:00:31  strk
 * Added new cleanup to Unload::Release
 *
 * Revision 1.5  2004/04/01 10:44:34  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.4  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.3  2004/03/19 09:48:46  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.2  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/unload.h>
#include <geos/opValid.h>
#include <geos/geomgraph.h>
#include <geos/operation.h>
#include <geos/opRelate.h>
#include <geos/indexQuadtree.h>
#include <geos/opBuffer.h>
#include <geos/planargraph.h>

namespace geos {

Unload::Unload(void)
{
}

Unload::~Unload(void)
{
}

void Unload::Release()
{
	delete PlanarGraph::cga;
	//delete planarDirectedEdge::cga;
	delete GeometryGraphOperation::cga;
	delete GeometryGraphOperation::li;
	delete RelateComputer::li;
	delete RelateComputer::ptLocator;
	delete QuadTreeRoot::origin;
	delete Geometry::INTERNAL_GEOMETRY_FACTORY;
}
}
