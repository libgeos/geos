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
 **********************************************************************
 * $Log$
 * Revision 1.45  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.44  2004/09/13 09:07:28  strk
 * Ported fix in LineString::isCoordinate
 *
 * Revision 1.43  2004/09/12 03:51:27  pramsey
 * Casting changes to allow OS/X compilation.
 *
 * Revision 1.42  2004/07/22 08:45:50  strk
 * Documentation updates, memory leaks fixed.
 *
 * Revision 1.41  2004/07/19 13:19:30  strk
 * Documentation fixes
 *
 * Revision 1.40  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.39  2004/07/06 17:58:22  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.38  2004/07/05 14:23:03  strk
 * More documentation cleanups.
 *
 * Revision 1.37  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.36  2004/07/03 12:51:37  strk
 * Documentation cleanups for DoxyGen.
 *
 * Revision 1.35  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.34  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.33  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.32  2004/06/15 20:38:44  strk
 * updated to respect deep-copy GeometryCollection interface
 *
 * Revision 1.31  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateSequence.
 *
 * Revision 1.30  2004/04/20 13:24:15  strk
 * More leaks removed.
 *
 * Revision 1.29  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.28  2004/04/10 22:41:24  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.27  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.26  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.25  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.24  2003/10/31 16:36:04  strk
 * Re-introduced clone() method. Copy constructor could not really replace it.
 *
 * Revision 1.23  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls to 
 * new getCoordinatesRO() when applicable.
 *
 * Revision 1.22  2003/10/15 09:54:29  strk
 * Added getCoordinatesRO() public method.
 *
 **********************************************************************/


#include <geos/geom.h>
#include <algorithm>
#include <typeinfo>
#include <geos/geosAlgorithm.h>
#include <geos/operation.h>

namespace geos {

//LineString::LineString(){}

LineString::LineString(const LineString &ls): Geometry(ls.getFactory()) {
	points=ls.points->clone();
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
LineString::LineString(CoordinateSequence *newCoords, const GeometryFactory *factory): Geometry(factory)
{
	if (newCoords==NULL) {
		points=factory->getCoordinateSequenceFactory()->create(NULL);
		return;
	}
	if (newCoords->getSize()==1) {
		throw new IllegalArgumentException("point array must contain 0 or >1 elements\n");
	}
	points=newCoords;
}


LineString::~LineString(){
	delete points;
}

Geometry* LineString::clone() const {
	return new LineString(*this);
}

CoordinateSequence* LineString::getCoordinates() const {
	return points->clone();
	//return points;
}

const CoordinateSequence* LineString::getCoordinatesRO() const {
	return points;
}

const Coordinate& LineString::getCoordinateN(int n) const {
	return points->getAt(n);
}

int LineString::getDimension() const {
	return 1;
}

int LineString::getBoundaryDimension() const {
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

bool LineString::isEmpty() const {
	return points->getSize()==0;
}

int LineString::getNumPoints() const {
	return points->getSize();
}

Point* LineString::getPointN(int n) const {
	return getFactory()->createPoint(points->getAt(n));
}

Point* LineString::getStartPoint() const {
	if (isEmpty()) {
		return new Point(NULL,NULL);
	}
	return getPointN(0);
}

Point* LineString::getEndPoint() const {
	if (isEmpty()) {
		return new Point(NULL,NULL);
	}
	return getPointN(getNumPoints() - 1);
}

bool LineString::isClosed() const {
	if (isEmpty()) {
		return false;
	}
	return getCoordinateN(0).equals2D(getCoordinateN(getNumPoints()-1));
}

bool LineString::isRing() const {
	return isClosed() && isSimple();
}

string LineString::getGeometryType() const {
	return "LineString";
}

bool LineString::isSimple() const {
	IsSimpleOp iso;
	Geometry *in = toInternalGeometry(this);
	bool issimple = iso.isSimple((LineString *)in);
	if ( (LineString *)in != this ) delete(in);
	return issimple;
}

Geometry* LineString::getBoundary() const {
	if (isEmpty()) {
		return getFactory()->createGeometryCollection(NULL);
	}
	if (isClosed()) {
		return getFactory()->createMultiPoint();
	}
	vector<Geometry*> *pts=new vector<Geometry*>();
	pts->push_back(getStartPoint());
	pts->push_back(getEndPoint());
	MultiPoint *mp = getFactory()->createMultiPoint(pts);
	//delete (*pts)[0];
	//delete (*pts)[1];
	//delete pts;
	return mp;
}

bool LineString::isCoordinate(Coordinate& pt) const {
	for (int i = 0; i < points->getSize(); i++) {
		if (points->getAt(i)==pt) {
			return true;
		}
	}
	return false;
}

Envelope* LineString::computeEnvelopeInternal() const {
	if (isEmpty()) {
		return new Envelope();
	}
	double minx = points->getAt(0).x;
	double miny = points->getAt(0).y;
	double maxx = points->getAt(0).x;
	double maxy = points->getAt(0).y;
	for (int i = 1; i < points->getSize(); i++) {
		minx = minx < points->getAt(i).x ? minx : points->getAt(i).x;
		maxx = maxx > points->getAt(i).x ? maxx : points->getAt(i).x;
		miny = miny < points->getAt(i).y ? miny : points->getAt(i).y;
		maxy = maxy > points->getAt(i).y ? maxy : points->getAt(i).y;
	}
	return new Envelope(minx, maxx, miny, maxy);
}

bool LineString::equalsExact(const Geometry *other, double tolerance) const {
	if (!isEquivalentClass(other)) {
		return false;
	}
	const LineString *otherLineString=dynamic_cast<const LineString*>(other);
	if (points->getSize()!=otherLineString->points->getSize()) {
		return false;
	}
	for (int i = 0; i < points->getSize(); i++) {
		if (!equal(points->getAt(i),otherLineString->points->getAt(i),tolerance)) {
			return false;
		}
	}
	return true;
}

void LineString::apply_rw(CoordinateFilter *filter)
{
	for (int i = 0; i < points->getSize(); i++) {
		Coordinate newcoord = points->getAt(i);
		filter->filter_rw(&newcoord);
		points->setAt(newcoord, i);
	}
}

void LineString::apply_ro(CoordinateFilter *filter) const {
	for (int i = 0; i < points->getSize(); i++) {
		// getAt returns a 'const' coordinate
		filter->filter_ro(&(points->getAt(i)));
	}
}

void LineString::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void LineString::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

/**
* Normalizes a LineString.  A normalized linestring
* has the first point which is not equal to it's reflected point
* less than the reflected point.
*/
void LineString::normalize() {
	for (int i = 0; i < points->getSize()/2; i++) {
		int j = points->getSize() - 1 - i;
		if (!(points->getAt(i)==points->getAt(j))) {
			if (points->getAt(i).compareTo(points->getAt(j)) > 0) {
				CoordinateSequence::reverse(points);
			}
			return;
		}
	}
}

int LineString::compareToSameClass(const Geometry *ls) const {
	LineString *line=(LineString*)ls;
	// MD - optimized implementation
	int i=0;
	int j=0;
	while(i<points->getSize() && j<line->points->getSize()) {
		int comparison=points->getAt(i).compareTo(line->points->getAt(j));
		if(comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<points->getSize()) {
		return 1;
	}
	if (j<line->points->getSize()) {
		return -1;
	}
	return 0;
//	return compare(*(points->toVector()),*(((LineString*)ls)->points->toVector()));
}

const Coordinate* LineString::getCoordinate() const
{
	// should use auto_ptr here or return NULL or throw an exception !
	// 	--strk;
	if (isEmpty()) return(new Coordinate());
	return &(points->getAt(0));
}

double LineString::getLength() const {
	return CGAlgorithms::length(points);
}

void LineString::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}

void LineString::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

int LineString::compareTo(const LineString *ls) const {
	if (isEmpty() && ls->isEmpty()) {
		return 0;
	}
	if (isEmpty()) {
		return -1;
	}
	if (ls->isEmpty()) {
		return 1;
	}
	return compareToSameClass(ls);
}

GeometryTypeId
LineString::getGeometryTypeId() const {
	return GEOS_LINESTRING;
}

}

