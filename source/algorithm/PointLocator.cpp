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
 * Last port: algorithm/PointLocator.java rev. 1.26 (JTS-1.7+)
 *
 **********************************************************************/

#include <cassert>
#include <typeinfo>

//#include <geos/geosAlgorithm.h>
//#include <geos/geomgraph.h>
//#include <geos/geom.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Location.h>
#include <geos/geomgraph/GeometryGraph.h>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm


int
PointLocator::locate(const Coordinate& p, const Geometry *geom)
{
	if (geom->isEmpty()) return Location::EXTERIOR;

	const LineString *ls_geom = dynamic_cast<const LineString *>(geom);
	if (ls_geom) return locate(p, ls_geom);

	const Polygon *poly_geom = dynamic_cast<const Polygon *>(geom);
	if (poly_geom) return locate(p, poly_geom);


	isIn=false;
	numBoundaries=0;
	computeLocation(p,geom);
	if (geomgraph::GeometryGraph::isInBoundary(numBoundaries)) return Location::BOUNDARY;
	if (numBoundaries>0 || isIn) return Location::INTERIOR;
	return Location::EXTERIOR;
}

/* private */
void
PointLocator::computeLocation(const Coordinate& p, const Geometry *geom)
{

	if (const LineString *ls=dynamic_cast<const LineString*>(geom))
	{
		updateLocationInfo(locate(p, ls));
	}
	else if (const Polygon *po=dynamic_cast<const Polygon*>(geom))
	{
		updateLocationInfo(locate(p, po));
	}
	else if (const MultiLineString *mls=dynamic_cast<const MultiLineString *>(geom))
	{
		for(int i=0, n=mls->getNumGeometries(); i<n; ++i)
		{
			const LineString *l=dynamic_cast<const LineString *>(mls->getGeometryN(i));
			updateLocationInfo(locate(p,l));
		}
	}
	else if (const MultiPolygon *mpo=dynamic_cast<const MultiPolygon *>(geom))
	{
		for(int i=0, n=mpo->getNumGeometries(); i<n; ++i)
		{
			const Polygon *po=dynamic_cast<const Polygon *>(mpo->getGeometryN(i));
			updateLocationInfo(locate(p, po));
		}
	}
	else if (const GeometryCollection *col=dynamic_cast<const GeometryCollection *>(geom))
	{
		for (GeometryCollection::const_iterator
				it=col->begin(), endIt=col->end();
				it != endIt;
				++it)
		{
			const Geometry *g2=*it;
			assert (g2!=geom); // is this check really needed ?
			computeLocation(p, g2);
		}
	}

}

/* private */
void
PointLocator::updateLocationInfo(int loc)
{
	if (loc==Location::INTERIOR) isIn=true;
	if (loc==Location::BOUNDARY) ++numBoundaries;
}

/* private */
int
PointLocator::locate(const Coordinate& p, const LineString *l)
{
	const CoordinateSequence* pt=l->getCoordinatesRO();
	if (! l->isClosed()) {
		if ((p==pt->getAt(0)) || (p==pt->getAt(pt->getSize()-1))) {
			return Location::BOUNDARY;
		}
	}
	if (CGAlgorithms::isOnLine(p,pt))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

/* private */
int
PointLocator::locateInPolygonRing(const Coordinate& p, const LinearRing *ring)
{
	// can this test be folded into isPointInRing ?

	const CoordinateSequence *cl = ring->getCoordinatesRO();

	if (CGAlgorithms::isOnLine(p,cl)) 
		return Location::BOUNDARY;
	if (CGAlgorithms::isPointInRing(p,cl))
		return Location::INTERIOR;
	return Location::EXTERIOR;
}

/* private */
int
PointLocator::locate(const Coordinate& p,const Polygon *poly)
{
	if (poly->isEmpty()) return Location::EXTERIOR;

	const LinearRing *shell=dynamic_cast<const LinearRing *>(poly->getExteriorRing());
	assert(shell);

	int shellLoc=locateInPolygonRing(p, shell);
	if (shellLoc==Location::EXTERIOR) return Location::EXTERIOR;
	if (shellLoc==Location::BOUNDARY) return Location::BOUNDARY;

	// now test if the point lies in or on the holes
	for(int i=0, n=poly->getNumInteriorRing(); i<n; ++i)
	{
		const LinearRing *hole=dynamic_cast<const LinearRing *>(poly->getInteriorRingN(i));
		int holeLoc=locateInPolygonRing(p,hole);
		if (holeLoc==Location::INTERIOR) return Location::EXTERIOR;
		if (holeLoc==Location::BOUNDARY) return Location::BOUNDARY;
	}
	return Location::INTERIOR;
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.28  2006/03/09 16:46:46  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.27  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.26  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.25  2006/01/31 19:07:33  strk
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
 * Revision 1.24  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.23  2005/06/24 11:09:42  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.22  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.21  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.20  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.19  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.18  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.16  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 * Revision 1.15  2003/10/15 15:47:43  strk
 * Adapted to new getCoordinatesRO() interface
 *
 **********************************************************************/

