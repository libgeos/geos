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

#include <geos/util/UnsupportedOperationException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>

#include <string>
#include <memory>

using namespace std;

namespace geos {
namespace geom { // geos::geom


/*protected*/
Point::Point(CoordinateSequence *newCoords, const GeometryFactory *factory)
	:
	Geometry(factory),
	coordinates(newCoords)
{
	if (coordinates.get()==NULL) {
		coordinates.reset(factory->getCoordinateSequenceFactory()->create(NULL));
		return;
	}        
	if (coordinates->getSize() != 1)
	{
		throw util::IllegalArgumentException("Point coordinate list must contain a single element");
	}
}

/*protected*/
Point::Point(const Point &p)
	:
	Geometry(p.getFactory()),
	coordinates(p.coordinates->clone())
{
}

CoordinateSequence *
Point::getCoordinates() const
{
	return coordinates->clone();
}

size_t
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

Dimension::DimensionType
Point::getDimension() const
{
	return Dimension::P; // point
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

Envelope::AutoPtr
Point::computeEnvelopeInternal() const
{
	if (isEmpty()) {
		return Envelope::AutoPtr(new Envelope());
	}

	return Envelope::AutoPtr(new Envelope(getCoordinate()->x,
			getCoordinate()->x, getCoordinate()->y,
			getCoordinate()->y));
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

void
Point::apply_rw(CoordinateSequenceFilter& filter)
{
	if (isEmpty()) return;
	filter.filter_rw(*coordinates, 0);
	if (filter.isGeometryChanged()) geometryChanged();
}

void
Point::apply_ro(CoordinateSequenceFilter& filter) const
{
	if (isEmpty()) return;
	filter.filter_ro(*coordinates, 0);
	//if (filter.isGeometryChanged()) geometryChanged();
}

bool
Point::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}

	// assume the isEquivalentClass would return false 
	// if other is not a point 
	assert(dynamic_cast<const Point*>(other));

	if ( isEmpty() ) return other->isEmpty();
	else if ( other->isEmpty() ) return false;

	const Coordinate* this_coord = getCoordinate();
	const Coordinate* other_coord = other->getCoordinate();

	// assume the isEmpty checks above worked :)
	assert(this_coord && other_coord);

	return equal(*this_coord, *other_coord, tolerance);
}

int
Point::compareToSameClass(const Geometry *point) const
{
	return getCoordinate()->compareTo(*(((Point*)point)->getCoordinate()));
}

Point::~Point()
{
	//delete coordinates;
}

GeometryTypeId
Point::getGeometryTypeId() const
{
	return GEOS_POINT;
}

/*public*/
const CoordinateSequence*
Point::getCoordinatesRO() const
{
	return coordinates.get();
}

} // namespace geos::geom
} // namesapce geos

/**********************************************************************
 *
 * $Log$
 * Revision 1.46  2006/05/04 15:49:39  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.45  2006/04/28 10:55:39  strk
 * Geometry constructors made protected, to ensure all constructions use GeometryFactory,
 * which has been made friend of all Geometry derivates. getNumPoints() changed to return
 * size_t.
 *
 * Revision 1.44  2006/04/10 18:15:09  strk
 * Changed Geometry::envelope member to be of type auto_ptr<Envelope>.
 * Changed computeEnvelopeInternal() signater to return auto_ptr<Envelope>
 *
 * Revision 1.43  2006/04/10 17:35:44  strk
 * Changed LineString::points and Point::coordinates to be wrapped
 * in an auto_ptr<>. This should close bugs #86 and #89
 *
 * Revision 1.42  2006/03/22 16:58:34  strk
 * Removed (almost) all inclusions of geom.h.
 * Removed obsoleted .cpp files.
 * Fixed a bug in WKTReader not using the provided CoordinateSequence
 * implementation, optimized out some memory allocations.
 *
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

