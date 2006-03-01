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

#include <geos/geosAlgorithm.h>
#include <geos/platform.h>
#include <typeinfo>

namespace geos {
namespace algorithm { // geos.algorithm

// file-statics
namespace {

double avg(double a, double b){return (a+b)/2.0;}

}


/*public*/
InteriorPointArea::InteriorPointArea(const Geometry *g)
{
	foundInterior=false;
	maxWidth=0.0;
	factory=g->getFactory();
	add(g);
}

/*public*/
InteriorPointArea::~InteriorPointArea()
{
}

/*public*/
bool 
InteriorPointArea::getInteriorPoint(Coordinate& ret) const
{
	if ( ! foundInterior ) return false;

	ret=interiorPoint;
	return true;
}

/*public*/
void
InteriorPointArea::add(const Geometry *geom)
{
	if (typeid(*geom)==typeid(Polygon)) {
		addPolygon(geom);
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

/*public*/
void
InteriorPointArea::addPolygon(const Geometry *geometry)
{
	LineString *bisector=horizontalBisector(geometry);
	Geometry *intersections=bisector->intersection(geometry);
	const Geometry *widestIntersection=widestGeometry(intersections);
	const Envelope *env=widestIntersection->getEnvelopeInternal();
	double width=env->getWidth();
	if (!foundInterior || width>maxWidth) {
		env->centre(interiorPoint);
		maxWidth = width;
		foundInterior=true;
	}
	//delete env;
	delete bisector;
	delete intersections;
}

//@return if geometry is a collection, the widest sub-geometry; otherwise,
//the geometry itself
const Geometry*
InteriorPointArea::widestGeometry(const Geometry *geometry)
{
	if ((typeid(*geometry)==typeid(GeometryCollection)) ||
				(typeid(*geometry)==typeid(MultiPoint)) ||
				(typeid(*geometry)==typeid(MultiPolygon)) ||
				(typeid(*geometry)==typeid(MultiLineString))) {
		return widestGeometry((GeometryCollection*) geometry);
	} else {
		return geometry;
	}
}

const Geometry*
InteriorPointArea::widestGeometry(const GeometryCollection* gc) {
	if (gc->isEmpty()) {
		return gc;
	}
	const Geometry* widestGeometry=gc->getGeometryN(0);
	for(int i=1;i<gc->getNumGeometries();i++) { //Start at 1
		const Envelope *env1(gc->getGeometryN(i)->getEnvelopeInternal());
		const Envelope *env2(widestGeometry->getEnvelopeInternal());
		if (env1->getWidth()>env2->getWidth()) {
				widestGeometry=gc->getGeometryN(i);
		}
	}
	return widestGeometry;
}

LineString*
InteriorPointArea::horizontalBisector(const Geometry *geometry)
{
	const Envelope *envelope=geometry->getEnvelopeInternal();
	// Assert: for areas, minx <> maxx
	double avgY=avg(envelope->getMinY(),envelope->getMaxY());

	vector<Coordinate>*cv=new vector<Coordinate>(2);
	(*cv)[0].x = envelope->getMinX();
	(*cv)[0].y = avgY;
	(*cv)[1].x = envelope->getMaxX();
	(*cv)[1].y = avgY;

	//delete envelope;

	CoordinateSequence *cl = factory->getCoordinateSequenceFactory()->create(cv);

	LineString *ret = factory->createLineString(cl);
	//delete cl;
	return ret;
}

} // namespace geos.algorithm
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.18  2006/03/01 17:16:31  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.17  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.16  2006/01/31 19:07:33  strk
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
 * Revision 1.15  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.14  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.13  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.12  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.11  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.10  2004/05/07 14:12:23  strk
 * Fixed segfault in destructor
 *
 * Revision 1.9  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.7  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 *
 **********************************************************************/

