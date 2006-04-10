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

#include <geos/util/IllegalArgumentException.h> 
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/operation/IsSimpleOp.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h> // for getBoundary
#include <geos/geom/Envelope.h>

#include <algorithm>
#include <typeinfo>
#include <memory>
#include <cassert>

using namespace std;
using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

LineString::LineString(const LineString &ls)
	:
	Geometry(ls.getFactory()),
	points(ls.points->clone())
{
	//points=ls.points->clone();
}

LineString*
LineString::reverse() const
{
	assert(points.get());
	CoordinateSequence* seq = points->clone();
	CoordinateSequence::reverse(seq);
	assert(getFactory());
	return getFactory()->createLineString(seq);
}


/**
 * Constructs a <code>LineString</code> taking ownership of the
 * given CoordinateSequence.
 *
 * @param newCoords the list of coordinates making up the linestring,
 *	or <code>null</code> to create the empty geometry.
 *	Consecutive points may not be equal.
 *
 * @param factory the GeometryFactory used to create this Geometry.
 *
 */  
LineString::LineString(CoordinateSequence *newCoords,
		const GeometryFactory *factory)
	:
	Geometry(factory),
	points(newCoords)
{
	if (points.get()==NULL) {
		points.reset(factory->getCoordinateSequenceFactory()->create(NULL));
		return;
	}

	if (points->size()==1)
	{
		throw util::IllegalArgumentException("point array must contain 0 or >1 elements\n");
	}
}


LineString::~LineString()
{
	//delete points;
}

CoordinateSequence*
LineString::getCoordinates() const
{
	assert(points.get());
	return points->clone();
	//return points;
}

const CoordinateSequence*
LineString::getCoordinatesRO() const
{
	return points.get();
}

const Coordinate&
LineString::getCoordinateN(int n) const
{
	assert(points.get());
	return points->getAt(n);
}

int
LineString::getDimension() const
{
	return 1; // change to Dimension::L
}

int
LineString::getBoundaryDimension() const
{
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

bool
LineString::isEmpty() const
{
	assert(points.get());
	return points->getSize()==0;
}

int
LineString::getNumPoints() const
{
	assert(points.get());
	return points->getSize();
}

Point*
LineString::getPointN(int n) const
{
	assert(getFactory());
	assert(points.get());
	return getFactory()->createPoint(points->getAt(n));
}

Point*
LineString::getStartPoint() const
{
	if (isEmpty()) {
		return NULL;
		//return new Point(NULL,NULL);
	}
	return getPointN(0);
}

Point*
LineString::getEndPoint() const
{
	if (isEmpty()) {
		return NULL;
		//return new Point(NULL,NULL);
	}
	return getPointN(getNumPoints() - 1);
}

bool
LineString::isClosed() const
{
	if (isEmpty()) {
		return false;
	}
	return getCoordinateN(0).equals2D(getCoordinateN(getNumPoints()-1));
}

bool
LineString::isRing() const
{
	return isClosed() && isSimple();
}

string
LineString::getGeometryType() const
{
	return "LineString";
}

bool
LineString::isSimple() const
{
	operation::IsSimpleOp iso;
	return iso.isSimple(this); 
}

Geometry*
LineString::getBoundary() const
{
	if (isEmpty()) {
		return getFactory()->createEmptyGeometry();
	}
	if (isClosed()) {
		return getFactory()->createMultiPoint();
	}
	vector<Geometry*> *pts=new vector<Geometry*>();
	pts->push_back(getStartPoint());
	pts->push_back(getEndPoint());
	MultiPoint *mp = getFactory()->createMultiPoint(pts);
	return mp;
}

bool
LineString::isCoordinate(Coordinate& pt) const
{
	assert(points.get());
	int npts=points->getSize();
	for (int i = 0; i<npts; i++) {
		if (points->getAt(i)==pt) {
			return true;
		}
	}
	return false;
}

/*protected*/
Envelope*
LineString::computeEnvelopeInternal() const
{
	if (isEmpty()) {
		// We don't return NULL here
		// as it would indicate "unknown"
		// envelope. In this case we
		// *know* the envelope is EMPTY.
		return new Envelope();
	}

	assert(points.get());
	const Coordinate& c=points->getAt(0);
	double minx = c.x;
	double miny = c.y;
	double maxx = c.x;
	double maxy = c.y;
	int npts=points->getSize();
	for (int i=1; i<npts; i++) {
		const Coordinate &c=points->getAt(i);
		minx = minx < c.x ? minx : c.x;
		maxx = maxx > c.x ? maxx : c.x;
		miny = miny < c.y ? miny : c.y;
		maxy = maxy > c.y ? maxy : c.y;
	}

	// caller expects a newly allocated Envelope.
	// this function won't be called twice, unless
	// cached Envelope is invalidated (set to NULL)
	return new Envelope(minx, maxx, miny, maxy);
}

bool
LineString::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}

	const LineString *otherLineString=dynamic_cast<const LineString*>(other);
	assert(otherLineString);
	unsigned int npts=points->getSize();
	if (npts!=otherLineString->points->getSize()) {
		return false;
	}
	for (unsigned int i=0; i<npts; ++i) {
		if (!equal(points->getAt(i),otherLineString->points->getAt(i),tolerance)) {
			return false;
		}
	}
	return true;
}

void
LineString::apply_rw(const CoordinateFilter *filter)
{
	assert(points.get());
	points->apply_rw(filter);
}

void
LineString::apply_ro(CoordinateFilter *filter) const
{
	assert(points.get());
	points->apply_ro(filter);
}

void LineString::apply_rw(GeometryFilter *filter)
{
	assert(filter);
	filter->filter_rw(this);
}

void LineString::apply_ro(GeometryFilter *filter) const
{
	assert(filter);
	filter->filter_ro(this);
}

/*public*/
void
LineString::normalize()
{
	assert(points.get());
	int npts=points->getSize();
	int n=npts/2;
	for (int i=0; i<n; i++) {
		int j = npts - 1 - i;
		if (!(points->getAt(i)==points->getAt(j))) {
			if (points->getAt(i).compareTo(points->getAt(j)) > 0) {
				CoordinateSequence::reverse(points.get());
			}
			return;
		}
	}
}

int
LineString::compareToSameClass(const Geometry *ls) const
{
	assert(dynamic_cast<const LineString*>(ls));
	const LineString *line=static_cast<const LineString*>(ls);
	// MD - optimized implementation
	int mynpts=points->getSize();
	int othnpts=line->points->getSize();
	if ( mynpts > othnpts ) return 1;
	if ( mynpts < othnpts ) return -1;
	for (int i=0; i<mynpts; i++)
	{
		int cmp=points->getAt(i).compareTo(line->points->getAt(i));
		if (cmp) return cmp;
	}
	return 0;
}

const Coordinate*
LineString::getCoordinate() const
{
	if (isEmpty()) return NULL; 
	return &(points->getAt(0));
}

double
LineString::getLength() const
{
	return CGAlgorithms::length(points.get());
}

void
LineString::apply_rw(GeometryComponentFilter *filter)
{
	assert(filter);
	filter->filter_rw(this);
}

void
LineString::apply_ro(GeometryComponentFilter *filter) const
{
	assert(filter);
	filter->filter_ro(this);
}

GeometryTypeId
LineString::getGeometryTypeId() const
{
	return GEOS_LINESTRING;
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.65  2006/04/10 17:35:44  strk
 * Changed LineString::points and Point::coordinates to be wrapped
 * in an auto_ptr<>. This should close bugs #86 and #89
 *
 * Revision 1.64  2006/04/05 10:25:20  strk
 * Fixed LineString constructor to ensure deletion of CoordinateSequence
 * argument on exception throw
 *
 * Revision 1.63  2006/03/31 16:55:17  strk
 * Added many assertions checking in LineString implementation.
 * Changed ::getCoordinate() to return NULL on empty geom.
 * Changed ::get{Start,End}Point() to return NULL on empty geom.
 *
 **********************************************************************/

