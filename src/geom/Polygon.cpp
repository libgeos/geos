/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@keybit.net>
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
 * Last port: geom/Polygon.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/algorithm/CGAlgorithms.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h> // for getBoundary()
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>

#include <vector>
#include <cmath> // for fabs
#include <cassert> 
#include <algorithm> 
#include <memory>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
//using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

/*protected*/
Polygon::Polygon(const Polygon &p)
	:
	Geometry(p)
{
	shell=new LinearRing(*p.shell);
	size_t nholes=p.holes->size();
	holes=new vector<Geometry *>(nholes);
	for(size_t i=0; i<nholes; ++i)
	{
    // TODO: holes is a vector of Geometry, anyway
    //       so there's no point in casting here,
    //       just use ->clone instead !
		const LinearRing* lr = dynamic_cast<const LinearRing *>((*p.holes)[i]);
		LinearRing *h=new LinearRing(*lr);
		(*holes)[i]=h;
	}
}

/*protected*/
Polygon::Polygon(LinearRing *newShell, vector<Geometry *> *newHoles,
		const GeometryFactory *newFactory):
	Geometry(newFactory)
{
	if (newShell==NULL) {
		shell=getFactory()->createLinearRing(NULL);
	}
	else
	{
		if (newHoles != NULL && newShell->isEmpty() && hasNonEmptyElements(newHoles)) {
			throw util::IllegalArgumentException("shell is empty but holes are not");
		}
		shell=newShell;
	}

	if (newHoles==NULL)
	{
		holes=new vector<Geometry *>();
	}
	else
	{
		if (hasNullElements(newHoles)) {
			throw util::IllegalArgumentException("holes must not contain null elements");
		}
		for (size_t i=0; i<newHoles->size(); i++)
			if ( (*newHoles)[i]->getGeometryTypeId() != GEOS_LINEARRING)
				throw util::IllegalArgumentException("holes must be LinearRings");
		holes=newHoles;
	}
}

CoordinateSequence*
Polygon::getCoordinates() const
{
	if (isEmpty()) {
		return getFactory()->getCoordinateSequenceFactory()->create(NULL);
	}

	vector<Coordinate> *cl = new vector<Coordinate>;

	// reserve space in the vector for all the polygon points
	cl->reserve(getNumPoints());

	// Add shell points
	const CoordinateSequence* shellCoords=shell->getCoordinatesRO();
	shellCoords->toVector(*cl);

	// Add holes points
	size_t nholes=holes->size();
	for (size_t i=0; i<nholes; ++i)
	{
		const LinearRing* lr = dynamic_cast<const LinearRing *>((*holes)[i]);
		const CoordinateSequence* childCoords = lr->getCoordinatesRO();
		childCoords->toVector(*cl);
	}

	return getFactory()->getCoordinateSequenceFactory()->create(cl);
}

size_t
Polygon::getNumPoints() const
{
	size_t numPoints = shell->getNumPoints();
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		const LinearRing* lr = dynamic_cast<const LinearRing *>((*holes)[i]);
		numPoints += lr->getNumPoints();
	}
	return numPoints;
}

Dimension::DimensionType
Polygon::getDimension() const
{
	return Dimension::A; // area
}

int
Polygon::getCoordinateDimension() const
{
	int dimension=2;

    if( shell != NULL )
        dimension = max(dimension,shell->getCoordinateDimension());

	size_t nholes=holes->size();
	for (size_t i=0; i<nholes; ++i)
	{
        dimension = max(dimension,(*holes)[i]->getCoordinateDimension());
	}

	return dimension;
}

int
Polygon::getBoundaryDimension() const
{
	return 1;
}

bool
Polygon::isEmpty() const
{
	return shell->isEmpty();
}

bool
Polygon::isSimple() const
{
	return true;
}

const LineString*
Polygon::getExteriorRing() const
{
	return shell;
}

size_t
Polygon::getNumInteriorRing() const
{
	return holes->size();
}

const LineString*
Polygon::getInteriorRingN(size_t n) const
{
  const LinearRing* lr = dynamic_cast<const LinearRing *>((*holes)[n]);
	return lr;
}

string
Polygon::getGeometryType() const
{
	return "Polygon";
}

// Returns a newly allocated Geometry object
/*public*/
Geometry*
Polygon::getBoundary() const
{
	/*
	 * We will make sure that what we
	 * return is composed of LineString,
	 * not LinearRings
	 */

	const GeometryFactory* gf = getFactory();

	if (isEmpty()) {
		return gf->createMultiLineString();
	}

	if ( ! holes->size() )
	{
		return gf->createLineString(*shell).release();
	}

	vector<Geometry *> *rings = new vector<Geometry *>(holes->size()+1);

	(*rings)[0] = gf->createLineString(*shell).release();
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		const LinearRing* hole = dynamic_cast<const LinearRing *>((*holes)[i]);
		assert( hole );
		LineString* ls = gf->createLineString( *hole ).release();
		(*rings)[i + 1] = ls;
	}
	MultiLineString *ret = getFactory()->createMultiLineString(rings);
	return ret;
}

Envelope::AutoPtr
Polygon::computeEnvelopeInternal() const
{
	return Envelope::AutoPtr(new Envelope(*(shell->getEnvelopeInternal())));
}

bool
Polygon::equalsExact(const Geometry *other, double tolerance) const
{
	const Polygon* otherPolygon=dynamic_cast<const Polygon*>(other);
	if ( ! otherPolygon ) return false;

	if (!shell->equalsExact(otherPolygon->shell, tolerance)) {
		return false;
	}

	size_t nholes = holes->size();

	if (nholes != otherPolygon->holes->size()) {
		return false;
	}

	for (size_t i=0; i<nholes; i++)
	{
		const Geometry* hole=(*holes)[i];
		const Geometry* otherhole=(*(otherPolygon->holes))[i];
		if (!hole->equalsExact(otherhole, tolerance))
		{
			return false;
		}
	}

	return true;
}

void
Polygon::apply_ro(CoordinateFilter *filter) const
{
	shell->apply_ro(filter);
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		const LinearRing* lr = dynamic_cast<const LinearRing *>((*holes)[i]);
		lr->apply_ro(filter);
	}
}

void
Polygon::apply_rw(const CoordinateFilter *filter)
{
	shell->apply_rw(filter);
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		LinearRing* lr = dynamic_cast<LinearRing *>((*holes)[i]);
		lr->apply_rw(filter);
	}
}

void
Polygon::apply_rw(GeometryFilter *filter)
{
	filter->filter_rw(this);
}

void
Polygon::apply_ro(GeometryFilter *filter) const
{
	filter->filter_ro(this);
}

Geometry*
Polygon::convexHull() const
{
	return getExteriorRing()->convexHull();
}

void
Polygon::normalize()
{
	normalize(shell, true);
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		LinearRing* lr = dynamic_cast<LinearRing *>((*holes)[i]);
		normalize(lr, false);
	}
	sort(holes->begin(), holes->end(), GeometryGreaterThen());
}

int
Polygon::compareToSameClass(const Geometry *g) const
{
	const Polygon* p = dynamic_cast<const Polygon*>(g);
	return shell->compareToSameClass(p->shell);
}

/*
 * TODO: check this function, there should be CoordinateSequence copy
 *       reduction possibility.
 */
void
Polygon::normalize(LinearRing *ring, bool clockwise)
{
	if (ring->isEmpty()) {
		return;
	}
	CoordinateSequence* uniqueCoordinates=ring->getCoordinates();
	uniqueCoordinates->deleteAt(uniqueCoordinates->getSize()-1);
	const Coordinate* minCoordinate=CoordinateSequence::minCoordinate(uniqueCoordinates);
	CoordinateSequence::scroll(uniqueCoordinates, minCoordinate);
	uniqueCoordinates->add(uniqueCoordinates->getAt(0));
	if (algorithm::CGAlgorithms::isCCW(uniqueCoordinates)==clockwise) {
		CoordinateSequence::reverse(uniqueCoordinates);
	}
	ring->setPoints(uniqueCoordinates);
	delete(uniqueCoordinates);
}

const Coordinate*
Polygon::getCoordinate() const
{
	return shell->getCoordinate();
}

/*
 *  Returns the area of this <code>Polygon</code>
 *
 * @return the area of the polygon
 */
double
Polygon::getArea() const
{
	double area=0.0;
	area+=fabs(algorithm::CGAlgorithms::signedArea(shell->getCoordinatesRO()));
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		const LinearRing *lr = dynamic_cast<const LinearRing *>((*holes)[i]);
		const CoordinateSequence *h=lr->getCoordinatesRO();
        	area-=fabs(algorithm::CGAlgorithms::signedArea(h));
	}
	return area;
}

/**
 * Returns the perimeter of this <code>Polygon</code>
 *
 * @return the perimeter of the polygon
 */
double
Polygon::getLength() const
{
	double len=0.0;
	len+=shell->getLength();
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		len+=(*holes)[i]->getLength();
	}
	return len;
}

void
Polygon::apply_ro(GeometryComponentFilter *filter) const
{
	filter->filter_ro(this);
	shell->apply_ro(filter);
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
        	(*holes)[i]->apply_ro(filter);
	}
}

void
Polygon::apply_rw(GeometryComponentFilter *filter)
{
	filter->filter_rw(this);
	shell->apply_rw(filter);
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
        	(*holes)[i]->apply_rw(filter);
	}
}

void
Polygon::apply_rw(CoordinateSequenceFilter& filter)
{
	shell->apply_rw(filter);

	if (! filter.isDone())
	{
		for (size_t i=0, n=holes->size(); i<n; ++i)
		{
			(*holes)[i]->apply_rw(filter);
			if (filter.isDone())
			break;
        	}
	}
	if (filter.isGeometryChanged()) geometryChanged();
}

void
Polygon::apply_ro(CoordinateSequenceFilter& filter) const
{
	shell->apply_ro(filter);

	if (! filter.isDone())
	{
		for (size_t i=0, n=holes->size(); i<n; ++i)
		{
			(*holes)[i]->apply_ro(filter);
			if (filter.isDone())
			break;
        	}
	}
	//if (filter.isGeometryChanged()) geometryChanged();
}

Polygon::~Polygon()
{
	delete shell;
	for(size_t i=0, n=holes->size(); i<n; ++i)
	{
		delete (*holes)[i];
	}
	delete holes;
}

GeometryTypeId
Polygon::getGeometryTypeId() const
{
	return GEOS_POLYGON;
}

bool
Polygon::isRectangle() const
{
	if ( getNumInteriorRing() != 0 ) return false;
	assert(shell!=NULL);
	if ( shell->getNumPoints() != 5 ) return false;

	const CoordinateSequence &seq = *(shell->getCoordinatesRO());

	// check vertices have correct values
	const Envelope &env = *getEnvelopeInternal();
	for (int i=0; i<5; i++) {
		double x = seq.getX(i);
		if (! (x == env.getMinX() || x == env.getMaxX())) return false;
		double y = seq.getY(i);
		if (! (y == env.getMinY() || y == env.getMaxY())) return false;
	}

	// check vertices are in right order
	double prevX = seq.getX(0);
	double prevY = seq.getY(0);
	for (int i = 1; i <= 4; i++) {
		double x = seq.getX(i);
		double y = seq.getY(i);
		bool xChanged = (x != prevX);
		bool yChanged = (y != prevY);
		if (xChanged == yChanged) return false;
		prevX = x;
		prevY = y;
	}
	return true;
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.68  2006/06/19 22:52:50  strk
 * optimized loops
 *
 * Revision 1.67  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.66  2006/06/08 17:58:57  strk
 * Polygon::getNumInteriorRing() return size_t, Polygon::interiorRingN() takes size_t.
 *
 * Revision 1.65  2006/05/04 15:49:39  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.64  2006/04/28 11:56:52  strk
 * * source/geom/GeometryFactory.cpp, source/headers/geos/geom/GeometryFactory.h: added LineString copy constructor.
 * * source/geom/Polygon.cpp: fixed getBoundary method to always return a geometry composed by LineStrings (not LinearRings)
 *
 * Revision 1.63  2006/04/28 10:55:39  strk
 * Geometry constructors made protected, to ensure all constructions use GeometryFactory,
 * which has been made friend of all Geometry derivates. getNumPoints() changed to return
 * size_t.
 *
 * Revision 1.62  2006/04/10 18:15:09  strk
 * Changed Geometry::envelope member to be of type auto_ptr<Envelope>.
 * Changed computeEnvelopeInternal() signater to return auto_ptr<Envelope>
 *
 * Revision 1.61  2006/03/15 09:13:36  strk
 * updated port info
 *
 * Revision 1.60  2006/03/10 10:34:58  strk
 * Added missing <algorithm> include (#54)
 *
 * Revision 1.59  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.58  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.57  2006/03/06 13:27:57  strk
 * Cleaned up equalsExact
 *
 * Revision 1.56  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.55  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.54  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.53  2006/02/02 02:20:15  strk
 * Fixed bug in isRectangle() failing to detect rectangles.
 *
 * Revision 1.52  2006/02/01 22:21:29  strk
 * - Added rectangle-based optimizations of intersects() and contains() ops
 * - Inlined all planarGraphComponent class
 *
 * Revision 1.51  2006/01/31 19:07:33  strk
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
 * Revision 1.50  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.49  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.48  2005/11/21 16:03:20  strk
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
 * Revision 1.47  2005/06/23 14:22:33  strk
 * Inlined and added missing ::clone() for Geometry subclasses
 *
 * Revision 1.46  2004/12/30 10:14:51  strk
 * never return LinearRing or MultiLinearRing from getBoundary
 *
 * Revision 1.45  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.44  2004/11/12 18:12:05  strk
 * Changed ::getBoundary() to return LineString if polygon has no holes.
 * (has required to pass OGC conformance test T20)
 *
 * Revision 1.43  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.42  2004/07/22 07:04:49  strk
 * Documented missing geometry functions.
 *
 * Revision 1.41  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.40  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.39  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.38  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.37  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.36  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.35  2004/06/28 21:58:24  strk
 * Constructors speedup.
 *
 * Revision 1.34  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.33  2004/06/15 20:30:47  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.32  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.31  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.30  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.29  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.28  2004/02/27 17:43:45  strk
 * memory leak fix in Polygon::getArea() - reported by 'Manuel  Prieto Villegas' <mprieto@dap.es>
 *
 * Revision 1.27  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.26  2003/10/31 16:36:04  strk
 * Re-introduced clone() method. Copy constructor could not really 
 * replace it.
 *
 * Revision 1.25  2003/10/17 05:51:21  ybychkov
 * Fixed a small memory leak.
 *
 * Revision 1.24  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 **********************************************************************/

