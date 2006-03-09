/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <string>

//#include <geos/geom.h>
//#include <geos/util.h>

#include <geos/util/UnsupportedOperationException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>

using namespace std;

namespace geos {
namespace geom { // geos::geom


/**
 * Creates a Point using the given CoordinateSequence (must have 1 element)
 *
 * @param  newCoords
 *	contains the single coordinate on which to base this
 *	<code>Point</code> or <code>null</code> to create
 *	the empty geometry.
 *
 *	If not null the created Point will take ownership of newCoords.
 */  
Point::Point(CoordinateSequence *newCoords, const GeometryFactory *factory):
	Geometry(factory)
{
	if (newCoords==NULL) {
		coordinates=factory->getCoordinateSequenceFactory()->create(NULL);
		return;
	}        
	if (newCoords->getSize() != 1)
	{
		throw util::IllegalArgumentException("Point coordinate list must contain a single element");
	}
	coordinates=newCoords;
}

Point::Point(const Point &p): Geometry(p.getFactory())
{
	coordinates=p.coordinates->clone();
}

CoordinateSequence *
Point::getCoordinates() const
{
	return coordinates->clone();
}

int
Point::getNumPoints() const
{
	return isEmpty() ? 0 : 1;
}

bool
Point::isEmpty() const
{
	return coordinates->isEmpty();
}

bool
Point::isSimple() const
{
	return true;
}

//bool Point::isValid() const {return true;}

int
Point::getDimension() const
{
	return 0;
}

int
Point::getBoundaryDimension() const
{
	return Dimension::False;
}

double
Point::getX() const
{
	if (isEmpty()) {
		throw util::UnsupportedOperationException("getX called on empty Point\n");
	}
	return getCoordinate()->x;
}

double
Point::getY() const
{
	if (isEmpty()) {
		throw util::UnsupportedOperationException("getY called on empty Point\n");
	}
	return getCoordinate()->y;
}

const Coordinate *
Point::getCoordinate() const
{
	return coordinates->getSize()!=0 ? &(coordinates->getAt(0)) : NULL;
}

string
Point::getGeometryType() const
{
	return "Point";
}

Geometry *
Point::getBoundary() const
{
	return getFactory()->createGeometryCollection(NULL);
}

Envelope *
Point::computeEnvelopeInternal() const
{
	if (isEmpty()) {
		return new Envelope();
	}
	return new Envelope(getCoordinate()->x, getCoordinate()->x, getCoordinate()->y, getCoordinate()->y);
}

void
Point::apply_ro(CoordinateFilter *filter) const
{
	if (isEmpty()) {return;}
	filter->filter_ro(getCoordinate());
}

void
Point::apply_rw(const CoordinateFilter *filter)
{
	if (isEmpty()) {return;}
	Coordinate newcoord = coordinates->getAt(0);
	filter->filter_rw(&newcoord);
	coordinates->setAt(newcoord, 0);
}

void
Point::apply_rw(GeometryFilter *filter)
{
	filter->filter_rw(this);
}

void
Point::apply_ro(GeometryFilter *filter) const
{
	filter->filter_ro(this);
}

void
Point::apply_rw(GeometryComponentFilter *filter)
{
	filter->filter_rw(this);
}

void
Point::apply_ro(GeometryComponentFilter *filter) const
{
	filter->filter_ro(this);
}

bool
Point::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}
	if (isEmpty() && other->isEmpty()) {
		return true;
	}
	bool ret = equal(*((Point*) other)->getCoordinate(), *getCoordinate(),
		tolerance);
	return ret;
}

int
Point::compareToSameClass(const Geometry *point) const
{
	return getCoordinate()->compareTo(*(((Point*)point)->getCoordinate()));
}

Point::~Point()
{
	delete coordinates;
}

GeometryTypeId
Point::getGeometryTypeId() const
{
	return GEOS_POINT;
}

} // namespace geos::geom
} // namesapce geos

/**********************************************************************
 *
 * $Log$
 * Revision 1.41  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.40  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.39  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.38  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.37  2006/01/31 19:07:33  strk
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
 * Revision 1.36  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.35  2005/06/23 14:22:33  strk
 * Inlined and added missing ::clone() for Geometry subclasses
 *
 * Revision 1.34  2005/05/23 16:42:43  strk
 * Added Refractions copyright
 *
 * Revision 1.33  2005/05/13 17:15:34  strk
 * cleanups and indentations
 *
 * Revision 1.32  2005/04/19 11:49:26  strk
 * Fixed segfault in ::isEmpty
 *
 * Revision 1.31  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.30  2004/09/13 12:39:14  strk
 * Made Point and MultiPoint subject to Validity tests.
 *
 * Revision 1.29  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 **********************************************************************/

