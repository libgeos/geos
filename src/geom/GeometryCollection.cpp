/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/GeometryCollection.java rev. 1.41
 *
 **********************************************************************/

#include <geos/geom/GeometryCollection.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/Envelope.h>

#ifndef GEOS_INLINE
# include <geos/geom/GeometryCollection.inl>
#endif

#include <algorithm>
#include <vector>
#include <memory>

using namespace std;

namespace geos {
namespace geom { // geos::geom

/*protected*/
GeometryCollection::GeometryCollection(const GeometryCollection &gc)
	:
	Geometry(gc)
{
	size_t ngeoms=gc.geometries->size();

	geometries=new vector<Geometry *>(ngeoms);
	for(size_t i=0; i<ngeoms; ++i)
	{
		(*geometries)[i]=(*gc.geometries)[i]->clone();
	}
}

/*protected*/
GeometryCollection::GeometryCollection(vector<Geometry *> *newGeoms, const GeometryFactory *factory):
	Geometry(factory)
{
	if (newGeoms==NULL) {
		geometries=new vector<Geometry *>();
		return;
	}
	if (hasNullElements(newGeoms)) {
		throw  util::IllegalArgumentException("geometries must not contain null elements\n");
		return;
	}
	geometries=newGeoms;
}

/*
 * Collects all coordinates of all subgeometries into a CoordinateSequence.
 * 
 * Returns a newly the collected coordinates
 *
 */
CoordinateSequence *
GeometryCollection::getCoordinates() const
{
	vector<Coordinate> *coordinates = new vector<Coordinate>(getNumPoints());

	int k = -1;
	for (size_t i=0; i<geometries->size(); ++i) {
		CoordinateSequence* childCoordinates=(*geometries)[i]->getCoordinates();
		size_t npts=childCoordinates->getSize();
		for (size_t j=0; j<npts; ++j) {
			k++;
			(*coordinates)[k] = childCoordinates->getAt(j);
		}
		delete childCoordinates; 
	}
	return CoordinateArraySequenceFactory::instance()->create(coordinates);
}

bool
GeometryCollection::isEmpty() const
{
	for (size_t i=0; i<geometries->size(); ++i) {
		if (!(*geometries)[i]->isEmpty()) {
			return false;
		}
	}
	return true;
}

Dimension::DimensionType
GeometryCollection::getDimension() const
{
	Dimension::DimensionType dimension=Dimension::False;
	for (size_t i=0, n=geometries->size(); i<n; ++i)
	{
		dimension=max(dimension,(*geometries)[i]->getDimension());
	}
	return dimension;
}

int
GeometryCollection::getBoundaryDimension() const
{
	int dimension=Dimension::False;
	for(size_t i=0; i<geometries->size(); ++i) {
		dimension=max(dimension,(*geometries)[i]->getBoundaryDimension());
	}
	return dimension;
}

int 
GeometryCollection::getCoordinateDimension() const
{
	int dimension=2;

	for (size_t i=0, n=geometries->size(); i<n; ++i)
	{
		dimension=max(dimension,(*geometries)[i]->getCoordinateDimension());
	}
	return dimension;
}

size_t
GeometryCollection::getNumGeometries() const
{
	return geometries->size();
}

const Geometry*
GeometryCollection::getGeometryN(size_t n) const
{
	return (*geometries)[n];
}

size_t
GeometryCollection::getNumPoints() const
{
	size_t numPoints = 0;
	for (size_t i=0, n=geometries->size(); i<n; ++i)
	{
		numPoints +=(*geometries)[i]->getNumPoints();
	}
	return numPoints;
}

string
GeometryCollection::getGeometryType() const
{
	return "GeometryCollection";
}

Geometry*
GeometryCollection::getBoundary() const
{
	throw util::IllegalArgumentException("Operation not supported by GeometryCollection\n");
}

bool
GeometryCollection::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) return false;

	const GeometryCollection* otherCollection=dynamic_cast<const GeometryCollection *>(other);
	if ( ! otherCollection ) return false;

	if (geometries->size()!=otherCollection->geometries->size()) {
		return false;
	}
	for (size_t i=0; i<geometries->size(); ++i) {
		if (!((*geometries)[i]->equalsExact((*(otherCollection->geometries))[i],tolerance)))
		{
			return false;
		}
	}
	return true;
}

void
GeometryCollection::apply_rw(const CoordinateFilter *filter)
{
	for (size_t i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_rw(filter);
	}
}

void
GeometryCollection::apply_ro(CoordinateFilter *filter) const
{
	for (size_t i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_ro(filter);
	}
}

void
GeometryCollection::apply_ro(GeometryFilter *filter) const
{
	filter->filter_ro(this);
	for(size_t i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_ro(filter);
	}
}

void
GeometryCollection::apply_rw(GeometryFilter *filter)
{
	filter->filter_rw(this);
	for(size_t i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_rw(filter);
	}
}

void
GeometryCollection::normalize()
{
	for (size_t i=0; i<geometries->size(); ++i) {
		(*geometries)[i]->normalize();
	}
	sort(geometries->begin(), geometries->end(), GeometryGreaterThen());
}

Envelope::AutoPtr
GeometryCollection::computeEnvelopeInternal() const
{
	Envelope::AutoPtr envelope(new Envelope());
	for (size_t i=0; i<geometries->size(); i++) {
		const Envelope *env=(*geometries)[i]->getEnvelopeInternal();
		envelope->expandToInclude(env);
	}
	return envelope;
}

int
GeometryCollection::compareToSameClass(const Geometry *g) const
{
  const GeometryCollection* gc = dynamic_cast<const GeometryCollection*>(g);
	return compare(*geometries, *(gc->geometries));
}

const Coordinate*
GeometryCollection::getCoordinate() const
{
	// should use auto_ptr here or return NULL or throw an exception !
	// 	--strk;
	if (isEmpty()) return new Coordinate();
    	return (*geometries)[0]->getCoordinate();
}

/**
 * @return the area of this collection
 */
double
GeometryCollection::getArea() const
{
	double area=0.0;
	for(size_t i=0; i<geometries->size(); ++i)
	{
        	area+=(*geometries)[i]->getArea();
	}
	return area;
}

/**
 * @return the total length of this collection
 */
double
GeometryCollection::getLength() const
{
	double sum=0.0;
	for(size_t i=0; i<geometries->size(); ++i)
	{
        	sum+=(*geometries)[i]->getLength();
	}
	return sum;
}

void
GeometryCollection::apply_rw(GeometryComponentFilter *filter)
{
	filter->filter_rw(this);
	for(size_t i=0; i<geometries->size(); ++i)
	{
        	(*geometries)[i]->apply_rw(filter);
	}
}

void
GeometryCollection::apply_ro(GeometryComponentFilter *filter) const
{
	filter->filter_ro(this);
	for(size_t i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_ro(filter);
	}
}

void
GeometryCollection::apply_rw(CoordinateSequenceFilter& filter)
{
	size_t ngeoms = geometries->size();
	if (ngeoms == 0 ) return;
	for (size_t i = 0; i < ngeoms; ++i)
	{
		(*geometries)[i]->apply_rw(filter);
		if (filter.isDone()) break;
	}
	if (filter.isGeometryChanged()) geometryChanged();
}

void
GeometryCollection::apply_ro(CoordinateSequenceFilter& filter) const
{
	size_t ngeoms = geometries->size();
	if (ngeoms == 0 ) return;
	for (size_t i = 0; i < ngeoms; ++i)
	{
		(*geometries)[i]->apply_ro(filter);
		if (filter.isDone()) break;
	}

	assert(!filter.isGeometryChanged()); // read-only filter...
	//if (filter.isGeometryChanged()) geometryChanged();
}

GeometryCollection::~GeometryCollection()
{
	for(size_t i=0; i<geometries->size(); ++i)
	{
		delete (*geometries)[i];
	}
	delete geometries;
}

GeometryTypeId
GeometryCollection::getGeometryTypeId() const
{
	return GEOS_GEOMETRYCOLLECTION;
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.60  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.59  2006/05/04 15:49:38  strk
 * updated all Geometry::getDimension() methods to return Dimension::DimensionType (closes bug#93)
 *
 * Revision 1.58  2006/04/28 10:55:39  strk
 * Geometry constructors made protected, to ensure all constructions use GeometryFactory,
 * which has been made friend of all Geometry derivates. getNumPoints() changed to return
 * size_t.
 *
 * Revision 1.57  2006/04/10 18:15:09  strk
 * Changed Geometry::envelope member to be of type auto_ptr<Envelope>.
 * Changed computeEnvelopeInternal() signater to return auto_ptr<Envelope>
 *
 * Revision 1.56  2006/04/07 09:54:30  strk
 * Geometry::getNumGeometries() changed to return 'unsigned int'
 * rather then 'int'
 *
 * Revision 1.55  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.54  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.53  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.52  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.51  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.50  2006/01/31 19:07:33  strk
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
 * Revision 1.49  2005/12/08 14:14:07  strk
 * ElevationMatrixFilter used for both elevation and Matrix fill,
 * thus removing CoordinateSequence copy in ElevetaionMatrix::add(Geometry *).
 * Changed CoordinateFilter::filter_rw to be a const method: updated
 * all apply_rw() methods to take a const CoordinateFilter.
 *
 * Revision 1.48  2005/11/25 09:57:51  strk
 * Fixed bug in getCoordinates() [ introduced by previous commit ]
 *
 * Revision 1.47  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.46  2005/11/21 16:03:20  strk
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
 * Revision 1.45  2005/06/23 14:22:32  strk
 * Inlined and added missing ::clone() for Geometry subclasses
 *
 * Revision 1.44  2004/12/08 14:32:54  strk
 * cleanups
 *
 * Revision 1.43  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.42  2004/07/22 08:45:50  strk
 * Documentation updates, memory leaks fixed.
 *
 * Revision 1.41  2004/07/22 07:04:49  strk
 * Documented missing geometry functions.
 *
 * Revision 1.40  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 **********************************************************************/

