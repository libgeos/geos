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

#include <vector>
#include <cassert>
#include <geos/geom.h>
#include <geos/operation.h>
#include <geos/geomgraph.h>
#include <geos/geosAlgorithm.h>

using namespace geos::algorithm;
using namespace geos::operation;
using namespace geos::geomgraph;

namespace geos {

/**
* Constructs a <code>MultiLineString</code>.
*
* @param  newLines
*	the <code>LineStrings</code>s for this
*	<code>MultiLineString</code>, or <code>null</code>
*	or an empty array to create the empty geometry.
*	Elements may be empty <code>LineString</code>s,
*	but not <code>null</code>s.
*
*	Constructed object will take ownership of
*	the vector and its elements.
*/
MultiLineString::MultiLineString(vector<Geometry *> *newLines, const GeometryFactory *factory): GeometryCollection(newLines,factory){}

MultiLineString::~MultiLineString(){}

int MultiLineString::getDimension() const {
	return 1;
}

int MultiLineString::getBoundaryDimension() const {
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

string MultiLineString::getGeometryType() const {
	return "MultiLineString";
}

bool MultiLineString::isClosed() const {
	if (isEmpty()) {
		return false;
	}
	for (unsigned int i = 0; i < geometries->size(); i++) {
		if (!((LineString *)(*geometries)[i])->isClosed()) {
			return false;
		}
	}
	return true;
}

bool MultiLineString::isSimple() const {
	IsSimpleOp iso;
	return iso.isSimple(this);
}

Geometry* MultiLineString::getBoundary() const {
	if (isEmpty()) {
		return getFactory()->createGeometryCollection(NULL);
	}
	//Geometry *in = toInternalGeometry(this);
	GeometryGraph gg(0, this);
	CoordinateSequence *pts=gg.getBoundaryPoints();
	//if ( (MultiLineString *)in != this ) delete(in);
	Geometry *ret = getFactory()->createMultiPoint(*pts);
	delete pts;
	return ret;
}

bool
MultiLineString::equalsExact(const Geometry *other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other, tolerance);
}
GeometryTypeId
MultiLineString::getGeometryTypeId() const {
	return GEOS_MULTILINESTRING;
}

MultiLineString*
MultiLineString::reverse() const
{
	unsigned int nLines = geometries->size();
	Geometry::NonConstVect *revLines = new Geometry::NonConstVect(nLines);
	for (unsigned int i=0; i<nLines; ++i)
	{
		assert(dynamic_cast<LineString*>((*geometries)[i]));
		LineString *iLS = static_cast<LineString*>((*geometries)[i]);
		(*revLines)[nLines-1-i] = iLS->reverse();
	}
	return getFactory()->createMultiLineString(revLines);
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.26  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.25  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
 * Revision 1.24  2006/01/31 19:07:33  strk
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
 * Revision 1.23  2004/09/12 03:51:27  pramsey
 * Casting changes to allow OS/X compilation.
 *
 * Revision 1.22  2004/07/22 08:45:50  strk
 * Documentation updates, memory leaks fixed.
 *
 * Revision 1.21  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.20  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.19  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.18  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.17  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.16  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.15  2004/05/07 13:04:57  strk
 * leak removed in MultiLineString::getBoundary()
 *
 * Revision 1.14  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateSequence.
 *
 * Revision 1.13  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.12  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

