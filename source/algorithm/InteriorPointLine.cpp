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
 **********************************************************************
 *
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <geos/geom.h> // for CoordinateSequence
#include <typeinfo>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace algorithm { // geos.algorithm

InteriorPointLine::InteriorPointLine(Geometry *g)
{
	minDistance=DoubleInfinity;
	hasInterior=false;
	if ( g->getCentroid(centroid) )
	{
#if GEOS_DEBUG
		std::cerr << "Centroid: " << centroid << std::endl;
#endif
		addInterior(g);
		if (!hasInterior) addEndpoints(g);
	}
}

InteriorPointLine::~InteriorPointLine()
{
}

/* private
 *
 * Tests the interior vertices (if any)
 * defined by a linear Geometry for the best inside point.
 * If a Geometry is not of dimension 1 it is not tested.
 * @param geom the geometry to add
 */
void
InteriorPointLine::addInterior(const Geometry *geom)
{
	const LineString *ls = dynamic_cast<const LineString*>(geom);
	if ( ls ) {
		addInterior(ls->getCoordinatesRO());
		return;
	}

	const GeometryCollection *gc = dynamic_cast<const GeometryCollection*>(geom);
	if ( gc )
	{
		for(unsigned int i=0, n=gc->getNumGeometries(); i<n; i++) {
			addInterior(gc->getGeometryN(i));
		}
	}
}

void
InteriorPointLine::addInterior(const CoordinateSequence *pts)
{
	unsigned int n=pts->getSize()-1;
	for(unsigned int i=1; i<n; ++i)
	{
		add(pts->getAt(i));
	}
}

/* private
 *
 * Tests the endpoint vertices
 * defined by a linear Geometry for the best inside point.
 * If a Geometry is not of dimension 1 it is not tested.
 * @param geom the geometry to add
 */
void
InteriorPointLine::addEndpoints(const Geometry *geom)
{
	const LineString *ls = dynamic_cast<const LineString*>(geom);
	if ( ls ) {
		addEndpoints(ls->getCoordinatesRO());
		return;
	}

	const GeometryCollection *gc = dynamic_cast<const GeometryCollection*>(geom);
	if ( gc )
	{
		for(unsigned int i=0, n=gc->getNumGeometries(); i<n; i++) {
			addEndpoints(gc->getGeometryN(i));
		}
	}
}

void
InteriorPointLine::addEndpoints(const CoordinateSequence *pts)
{
	add(pts->getAt(0));
	add(pts->getAt(pts->getSize()-1));
}

/*private*/
void
InteriorPointLine::add(const Coordinate& point)
{

	double dist=point.distance(centroid);
#if GEOS_DEBUG
	std::cerr << "point " << point << " dist " << dist << ", minDistance " << minDistance << std::endl;
#endif
	if (!hasInterior || dist<minDistance) {
		interiorPoint=point;
#if GEOS_DEBUG
		std::cerr << " is new InteriorPoint" << std::endl;
#endif
		minDistance=dist;
		hasInterior=true;
	}
}

bool
InteriorPointLine::getInteriorPoint(Coordinate& ret) const
{
	if ( ! hasInterior ) return false;
	ret=interiorPoint;
	return true;
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/03/03 13:50:15  strk
 * Cleaned up InteriorPointLine class
 *
 * Revision 1.15  2006/03/01 18:36:56  strk
 * Geometry::createPointFromInternalCoord dropped (it's a duplication of GeometryFactory::createPointFromInternalCoord).
 * Fixed bugs in InteriorPoint* and getCentroid() inserted by previous commits.
 *
 * Revision 1.14  2006/03/01 17:16:31  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.13  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.12  2006/01/31 19:07:33  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.11  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.10  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.9  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.7  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 **********************************************************************/

