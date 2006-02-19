/**********************************************************************
 * $Id$
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
 **********************************************************************/

#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <typeinfo>

namespace geos {
namespace algorithm { // geos.algorithm

CentroidLine::CentroidLine():
	totalLength(0.0)
{
}

CentroidLine::~CentroidLine()
{
}

/**
 * Adds the linestring(s) defined by a Geometry to the centroid total.
 * If the geometry is not linear it does not contribute to the centroid
 * @param geom the geometry to add
 */
void
CentroidLine::add(const Geometry *geom)
{
	if (typeid(*geom)==typeid(LineString) || typeid(*geom)==typeid(LinearRing)) {
		add(((LineString*)geom)->getCoordinatesRO());
	} else if ((typeid(*geom)==typeid(GeometryCollection)) ||
				(typeid(*geom)==typeid(MultiPoint)) ||
				(typeid(*geom)==typeid(MultiPolygon)) ||
				(typeid(*geom)==typeid(MultiLineString))) {
		GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			add(gc->getGeometryN(i));
		}
	}
}

/**
 * Adds the length defined by an array of coordinates.
 * @param pts an array of {@link Coordinate}s
 */
void
CentroidLine::add(const CoordinateSequence *pts)
{
	unsigned int npts=pts->getSize();

	for(unsigned int i=1; i<npts; ++i)
	{
		const Coordinate &p1=pts->getAt(i-1);
		const Coordinate &p2=pts->getAt(i);

		double segmentLen=p1.distance(p2);
		totalLength+=segmentLen;
		double midx=(p1.x+p2.x)/2;
		centSum.x+=segmentLen*midx;
		double midy=(p1.y+p2.y)/2;
		centSum.y+=segmentLen*midy;
	}
}

Coordinate *
CentroidLine::getCentroid() const
{
	return new Coordinate(centSum.x/totalLength, centSum.y/totalLength);
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
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
 * Revision 1.11  2005/11/24 23:07:00  strk
 * CentroidLine made concrete class (only destructor was virtual) - avoided heap allocation for owned Coordinate centSum
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
 * Revision 1.7  2003/10/29 10:38:26  strk
 * Added support for LinearRing types (treated as LineString)
 *
 * Revision 1.6  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 **********************************************************************/

