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
 **********************************************************************/

#include <string>

#include <geos/geom/LinearRing.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/IllegalArgumentException.h>

using namespace std;

namespace geos {
namespace geom { // geos::geom

LinearRing::LinearRing(const LinearRing &lr): LineString(lr) {}

/**
*  Constructs a <code>LinearRing</code> with the given points.
*
*@param  points          points forming a closed and simple linestring, or
*      <code>null</code> or an empty array to create the empty geometry.
*      This array must not contain <code>null</code> elements.
*	The created LinearRing will take ownership of points.
*
*/
LinearRing::LinearRing(CoordinateSequence* newCoords, const GeometryFactory *newFactory): LineString(newCoords,newFactory) {
	validateConstruction();	
}


void LinearRing::validateConstruction() {
	if (!LineString::isEmpty() && !LineString::isClosed()) {
		throw util::IllegalArgumentException("points must form a closed linestring");
    }
	if (!points->isEmpty() && (points->getSize()>=1 && points->getSize()<=3)) {
		throw util::IllegalArgumentException("Number of points must be 0 or >3");
	}
}


		
// superclass LineString will delete internal CoordinateSequence
LinearRing::~LinearRing(){
}

bool LinearRing::isSimple() const {
	return true;
}
string LinearRing::getGeometryType() const {
	return "LinearRing";
}
bool LinearRing::isClosed() const {
	return true;
}

void LinearRing::setPoints(CoordinateSequence* cl){
	const vector<Coordinate> *v=cl->toVector();
	points->setPoints(*(v));
	//delete v;
}

GeometryTypeId
LinearRing::getGeometryTypeId() const {
	return GEOS_LINEARRING;
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.30  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.29  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.28  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.27  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.26  2006/01/31 19:07:33  strk
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
 * Revision 1.25  2005/06/23 14:22:33  strk
 * Inlined and added missing ::clone() for Geometry subclasses
 *
 * Revision 1.24  2004/12/16 16:27:24  strk
 * Fixed LinearRing::clone() to return LinearRing instead of LineString
 *
 * Revision 1.23  2004/12/03 22:52:56  strk
 * enforced const return of CoordinateSequence::toVector() method to derivate classes.
 *
 * Revision 1.22  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.21  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.20  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.19  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.18  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.17  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.16  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.15  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.14  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.13  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.12  2003/10/11 03:23:22  strk
 * fixed spurious typos
 *
 **********************************************************************/

