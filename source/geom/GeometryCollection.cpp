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

#include <geos/geom.h>
#include <geos/util.h>
#include <geos/geosAlgorithm.h>
#include <algorithm>
#include <vector>
#include <typeinfo>

using namespace std;

namespace geos {

GeometryCollection::GeometryCollection(const GeometryCollection &gc):
	Geometry(gc.getFactory())
{
	unsigned int ngeoms=gc.geometries->size();

	geometries=new vector<Geometry *>(ngeoms);
	for(unsigned int i=0; i<ngeoms; ++i)
	{
		(*geometries)[i]=(*gc.geometries)[i]->clone();
	}
}

/**
 * @param newGeoms
 *	the <code>Geometry</code>s for this
 *	<code>GeometryCollection</code>,
 *	or <code>null</code> or an empty array to
 *	create the empty geometry.
 *	Elements may be empty <code>Geometry</code>s,
 *	but not <code>null</code>s.
 *
 *	If construction succeed the created object will take
 *	ownership of newGeoms vector and elements.
 *
 *	If construction	fails "IllegalArgumentException *"
 *	is thrown and it is your responsibility to delete newGeoms
 *	vector and content.
 *
 */
GeometryCollection::GeometryCollection(vector<Geometry *> *newGeoms, const GeometryFactory *factory):
	Geometry(factory)
{
	if (newGeoms==NULL) {
		geometries=new vector<Geometry *>();
		return;
	}
	if (hasNullElements(newGeoms)) {
		throw  IllegalArgumentException("geometries must not contain null elements\n");
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
	for (unsigned int i=0; i<geometries->size(); ++i) {
		CoordinateSequence* childCoordinates=(*geometries)[i]->getCoordinates();
		unsigned int npts=childCoordinates->getSize();
		for (unsigned int j=0; j<npts; ++j) {
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
	for (unsigned int i=0; i<geometries->size(); ++i) {
		if (!(*geometries)[i]->isEmpty()) {
			return false;
		}
	}
	return true;
}

int
GeometryCollection::getDimension() const
{
	int dimension=Dimension::False;
	for (unsigned int i=0; i<geometries->size(); ++i) {
		dimension=max(dimension,(*geometries)[i]->getDimension());
	}
	return dimension;
}

int
GeometryCollection::getBoundaryDimension() const
{
	int dimension=Dimension::False;
	for(unsigned int i=0; i<geometries->size(); ++i) {
		dimension=max(dimension,(*geometries)[i]->getBoundaryDimension());
	}
	return dimension;
}

int
GeometryCollection::getNumGeometries() const
{
	return (int)geometries->size();
}

const Geometry*
GeometryCollection::getGeometryN(int n) const
{
	return (*geometries)[n];
}

int
GeometryCollection::getNumPoints() const
{
	int numPoints = 0;
	for (unsigned int i=0; i<geometries->size(); ++i)
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

bool
GeometryCollection::isSimple() const
{
	throw  IllegalArgumentException("This method is not supported by GeometryCollection objects\n");
	return false;
}

Geometry*
GeometryCollection::getBoundary() const
{
	throw  IllegalArgumentException("This method is not supported by GeometryCollection objects\n");
	return NULL;
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
	for (unsigned int i=0; i<geometries->size(); ++i) {
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
	for (unsigned int i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_rw(filter);
	}
}

void
GeometryCollection::apply_ro(CoordinateFilter *filter) const
{
	for (unsigned int i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_ro(filter);
	}
}

void
GeometryCollection::apply_ro(GeometryFilter *filter) const
{
	filter->filter_ro(this);
	for(unsigned int i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_ro(filter);
	}
}

void
GeometryCollection::apply_rw(GeometryFilter *filter)
{
	filter->filter_rw(this);
	for(unsigned int i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_rw(filter);
	}
}

void
GeometryCollection::normalize()
{
	for (unsigned int i=0; i<geometries->size(); ++i) {
		(*geometries)[i]->normalize();
	}
	sort(geometries->begin(), geometries->end(), GeometryGreaterThen());
}

Envelope*
GeometryCollection::computeEnvelopeInternal() const
{
	Envelope* envelope=new Envelope();
	for (unsigned int i=0; i<geometries->size(); i++) {
		//Envelope *env=new Envelope(*((*geometries)[i]->getEnvelopeInternal()));
		const Envelope *env=(*geometries)[i]->getEnvelopeInternal();
		envelope->expandToInclude(env);
	}
	return envelope;
}

int
GeometryCollection::compareToSameClass(const Geometry *gc) const
{
	return compare(*geometries, *(((GeometryCollection*)gc)->geometries));
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
	for(unsigned int i=0; i<geometries->size(); ++i)
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
	for(unsigned int i=0; i<geometries->size(); ++i)
	{
        	sum+=(*geometries)[i]->getLength();
	}
	return sum;
}

void
GeometryCollection::apply_rw(GeometryComponentFilter *filter)
{
	filter->filter_rw(this);
	for(unsigned int i=0; i<geometries->size(); ++i)
	{
        	(*geometries)[i]->apply_rw(filter);
	}
}

void
GeometryCollection::apply_ro(GeometryComponentFilter *filter) const
{
	filter->filter_ro(this);
	for(unsigned int i=0; i<geometries->size(); ++i)
	{
		(*geometries)[i]->apply_ro(filter);
	}
}

GeometryCollection::~GeometryCollection()
{
	for(unsigned int i=0; i<geometries->size(); ++i)
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

} // namespace geos

/**********************************************************************
 * $Log$
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

