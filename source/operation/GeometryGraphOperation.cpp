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

#include <geos/operation.h>
#include <geos/geosAlgorithm.h>
#include <geos/geomgraph.h>

using namespace geos::algorithm;
using namespace geos::geomgraph;

namespace geos {
namespace operation { // geos.operation

//LineIntersector* GeometryGraphOperation::li=new LineIntersector();

GeometryGraphOperation::GeometryGraphOperation(const Geometry *g0, const Geometry *g1):
	arg(2)
{
	// use the most precise model for the result
	if (g0->getPrecisionModel()->compareTo(g1->getPrecisionModel())>=0)
		setComputationPrecision(g0->getPrecisionModel());
	else
		setComputationPrecision(g1->getPrecisionModel());
	arg[0]=new GeometryGraph(0, g0);
	arg[1]=new GeometryGraph(1, g1);
}


GeometryGraphOperation::GeometryGraphOperation(const Geometry *g0):
	arg(1)
{
	setComputationPrecision(g0->getPrecisionModel());
	arg[0]=new GeometryGraph(0, g0);
}

const Geometry*
GeometryGraphOperation::getArgGeometry(int i) const
{
	return arg[i]->getGeometry();
}

/*protected*/
void
GeometryGraphOperation::setComputationPrecision(const PrecisionModel* pm)
{
    resultPrecisionModel=pm;
    li.setPrecisionModel(resultPrecisionModel);
}

GeometryGraphOperation::~GeometryGraphOperation()
{
	for(unsigned int i=0; i<arg.size(); ++i)
	{
		delete arg[i];
	}
}

} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.21  2006/03/02 14:34:43  strk
 * GeometryGraphOperation::li made a non-static member, and not more a pointer
 *
 * Revision 1.20  2006/02/27 09:05:33  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.19  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.18  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.17  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.16  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.15  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.14  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.13  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

