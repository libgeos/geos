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
 * Revision 1.41  2004/07/13 08:33:52  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
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


#include <geos/geom.h>
#include <typeinfo>
#include <geos/geosAlgorithm.h>

namespace geos {

Polygon::Polygon(const Polygon &p): Geometry(p.getFactory()){
	shell=new LinearRing(*p.shell);
	holes=new vector<Geometry *>();
	for(int i=0;i<(int)p.holes->size();i++) {
		LinearRing *h=new LinearRing(* (LinearRing*)(*p.holes)[i]);
		holes->push_back(h);
	}
}

/**
* Constructs a <code>Polygon</code> with the given exterior
* and interior boundaries.
*
* @param  shell     the outer boundary of the new <code>Polygon</code>,
*                   or <code>null</code> or an empty
*                   <code>LinearRing</code> if the empty geometry
*                   is to be created.
*
* @param  holes     the <code>LinearRings</code> defining the inner
*                   boundaries of the new <code>Polygon</code>, or
*                   <code>null</code> or empty <code>LinearRing</code>s
*                   if the empty  geometry is to be created.
*
* Polygon will take ownership of Shell and Holes LinearRings
*
*/
Polygon::Polygon(LinearRing *newShell, vector<Geometry *> *newHoles, const GeometryFactory *newFactory): Geometry(newFactory)
{
	if (newShell==NULL) {
		shell=getFactory()->createLinearRing(NULL);
	}
	else
	{
		if (newShell->isEmpty() && hasNonEmptyElements(newHoles)) {
			delete newShell;
			delete newHoles;
			throw new IllegalArgumentException("shell is empty but holes are not");
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
			delete newShell;
			delete newHoles;
			throw new IllegalArgumentException("holes must not contain null elements");
		}
		for (unsigned int i=0; i<newHoles->size(); i++)
			if ( (*newHoles)[i]->getGeometryTypeId() != GEOS_LINEARRING)
				throw new IllegalArgumentException("holes must be LinearRings");
		holes=newHoles;
	}
}

Geometry *Polygon::clone() const {
	return new Polygon(*this);
}

CoordinateSequence* Polygon::getCoordinates() const {
	if (isEmpty()) {
		return getFactory()->getCoordinateSequenceFactory()->create(NULL);
	}

	vector<Coordinate> *cl = new vector<Coordinate>;

	int k = -1;
	const CoordinateSequence* shellCoords=shell->getCoordinatesRO();
	for (int x = 0; x < shellCoords->getSize(); x++) {
		k++;
		cl->push_back(shellCoords->getAt(x));
	}

	for (unsigned int i = 0; i < holes->size(); i++) {
		const CoordinateSequence* childCoords=((LinearRing *)(*holes)[i])->getCoordinatesRO();
		for (int j = 0; j < childCoords->getSize(); j++) {
			k++;
			cl->push_back(childCoords->getAt(j));
		}
	}

	return getFactory()->getCoordinateSequenceFactory()->create(cl);
}

int Polygon::getNumPoints() const {
	int numPoints = shell->getNumPoints();
	for (unsigned int i = 0; i < holes->size(); i++) {
		numPoints += ((LinearRing *)(*holes)[i])->getNumPoints();
	}
	return numPoints;
}

int Polygon::getDimension() const {
	return 2;
}

int Polygon::getBoundaryDimension() const {
	return 1;
}

bool Polygon::isEmpty() const {
	return shell->isEmpty();
}

bool Polygon::isSimple() const {
	return true;
}

const LineString* Polygon::getExteriorRing() const {
	return shell;
}

int Polygon::getNumInteriorRing() const {
	return (int)holes->size();
}

const LineString* Polygon::getInteriorRingN(int n) const {
	return (LineString *) (*holes)[n];
}

string Polygon::getGeometryType() const {
	return "Polygon";
}

// Returns a newly allocated Geometry object
Geometry* Polygon::getBoundary() const {
	if (isEmpty()) {
		return getFactory()->createGeometryCollection(NULL);
	}
	vector<Geometry *> rings(holes->size()+1);
	rings[0]=shell;
	for (unsigned int i=0; i<holes->size(); i++) {
		rings[i + 1] = (*holes)[i];
	}
	MultiLineString *ret =getFactory()->createMultiLineString(rings);
	return ret;
}

Envelope* Polygon::computeEnvelopeInternal() const {
	return shell->getEnvelopeInternal();
}

bool Polygon::equalsExact(const Geometry *other, double tolerance) const {
	if (!isEquivalentClass(other)) {
		return false;
	}
	const Polygon* otherPolygon=dynamic_cast<const Polygon*>(other);
	Geometry* thisShell=dynamic_cast<Geometry *>(shell);
	if (typeid(*(otherPolygon->shell))!=typeid(Geometry)) {
		return false;
	}
	Geometry* otherPolygonShell=dynamic_cast<Geometry *>(otherPolygon->shell);
	if (!shell->equalsExact(otherPolygonShell, tolerance)) {
		return false;
	}
	if (holes->size()!=otherPolygon->holes->size()) {
		return false;
	}
	for (unsigned int i = 0; i < holes->size(); i++) {
		if (!((LinearRing *)(*holes)[i])->equalsExact((*(otherPolygon->holes))[i],tolerance)) {
			return false;
		}
	}
	return true;
}

void Polygon::apply_ro(CoordinateFilter *filter) const {
	shell->apply_ro(filter);
	for (unsigned int i = 0; i < holes->size(); i++) {
		((LinearRing *)(*holes)[i])->apply_ro(filter);
	}
}

void Polygon::apply_rw(CoordinateFilter *filter) {
	shell->apply_rw(filter);
	for (unsigned int i = 0; i < holes->size(); i++) {
		((LinearRing *)(*holes)[i])->apply_rw(filter);
	}
}

void Polygon::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Polygon::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

Geometry* Polygon::convexHull() const {
	return getExteriorRing()->convexHull();
}

void Polygon::normalize() {
	normalize(shell, true);
	for (unsigned int i = 0; i < holes->size(); i++) {
		normalize((LinearRing *)(*holes)[i], false);
	}
	sort(holes->begin(),holes->end(),greaterThen);
}

int Polygon::compareToSameClass(const Geometry *p) const {
	return shell->compareToSameClass(((Polygon*)p)->shell);
}

void Polygon::normalize(LinearRing *ring, bool clockwise) {
	if (ring->isEmpty()) {
		return;
	}
	CoordinateSequence* uniqueCoordinates=ring->getCoordinates();
	uniqueCoordinates->deleteAt(uniqueCoordinates->getSize()-1);
	const Coordinate* minCoordinate=CoordinateSequence::minCoordinate(uniqueCoordinates);
	CoordinateSequence::scroll(uniqueCoordinates, minCoordinate);
	uniqueCoordinates->add(uniqueCoordinates->getAt(0));
	if (CGAlgorithms::isCCW(uniqueCoordinates)==clockwise) {
		CoordinateSequence::reverse(uniqueCoordinates);
	}
	ring->setPoints(uniqueCoordinates);
	delete(uniqueCoordinates);
}

const Coordinate* Polygon::getCoordinate() const {
	return shell->getCoordinate();
}

/**
*  Returns the area of this <code>Polygon</code>
*
*@return the area of the polygon
*/
double Polygon::getArea() const {
	double area=0.0;
	area+=fabs(CGAlgorithms::signedArea(shell->getCoordinatesRO()));
	for(unsigned int i=0;i<holes->size();i++) {
		CoordinateSequence *h=(*holes)[i]->getCoordinates();
        	area-=fabs(CGAlgorithms::signedArea(h));
		delete h;
	}
	return area;
}

/**
*  Returns the perimeter of this <code>Polygon</code>
*
*@return the perimeter of the polygon
*/
double Polygon::getLength() const {
	double len=0.0;
	len+=shell->getLength();
	for(unsigned int i=0;i<holes->size();i++) {
        len+=(*holes)[i]->getLength();
	}
	return len;
}

void Polygon::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
	shell->apply_ro(filter);
	for(unsigned int i=0;i<holes->size();i++) {
        	(*holes)[i]->apply_ro(filter);
	}
}

void Polygon::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
	shell->apply_rw(filter);
	for(unsigned int i=0;i<holes->size();i++) {
        	(*holes)[i]->apply_rw(filter);
	}
}

Polygon::~Polygon(){
	delete shell;
	for(int i=0;i<(int)holes->size();i++) {
		delete (*holes)[i];
	}
	delete holes;
}
GeometryTypeId
Polygon::getGeometryTypeId() const {
	return GEOS_POLYGON;
}
}

