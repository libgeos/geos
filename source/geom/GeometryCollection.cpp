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

#include <geos/geom.h>
#include <geos/util.h>
#include <geos/geosAlgorithm.h>
#include <algorithm>
#include <typeinfo>

namespace geos {

GeometryCollection::GeometryCollection(const GeometryCollection &gc):
	Geometry(gc.getFactory()){
	geometries=new vector<Geometry *>();
	for(int i=0;i<(int)gc.geometries->size();i++) {
		geometries->push_back((*gc.geometries)[i]->clone());
	}
	//geometries=gc.geometries;	
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
GeometryCollection::GeometryCollection(vector<Geometry *> *newGeoms, const GeometryFactory *factory): Geometry(factory)
{
	if (newGeoms==NULL) {
		geometries=new vector<Geometry *>();
		return;
	}
	if (hasNullElements(newGeoms)) {
		throw new IllegalArgumentException("geometries must not contain null elements\n");
		return;
	}
	geometries=newGeoms;
}

Geometry* GeometryCollection::clone() const {
	return new GeometryCollection(*this);
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
	for (unsigned int i=0; i<geometries->size(); i++) {
		CoordinateSequence* childCoordinates=(*geometries)[i]->getCoordinates();
		for (int j=0; j<childCoordinates->getSize(); j++) {
			k++;
			(*coordinates)[k] = childCoordinates->getAt(j);
		}
		delete childCoordinates; 
	}
	return DefaultCoordinateSequenceFactory::instance()->create(coordinates);
}

bool GeometryCollection::isEmpty() const {
	for (unsigned int i=0; i<geometries->size(); i++) {
		if (!(*geometries)[i]->isEmpty()) {
			return false;
		}
	}
	return true;
}

int GeometryCollection::getDimension() const {
	int dimension=Dimension::False;
	for (unsigned int i=0; i<geometries->size(); i++) {
		dimension=max(dimension,(*geometries)[i]->getDimension());
	}
	return dimension;
}

int GeometryCollection::getBoundaryDimension() const {
	int dimension=Dimension::False;
	for(unsigned int i=0; i<geometries->size(); i++) {
		dimension=max(dimension,(*geometries)[i]->getBoundaryDimension());
	}
	return dimension;
}

int GeometryCollection::getNumGeometries() const {
	return (int)geometries->size();
}

const Geometry* GeometryCollection::getGeometryN(int n) const {
	return (*geometries)[n];
}

int GeometryCollection::getNumPoints() const {
	int numPoints = 0;
	for (unsigned int i=0; i<geometries->size(); i++) {
		numPoints +=(*geometries)[i]->getNumPoints();
	}
	return numPoints;
}

string GeometryCollection::getGeometryType() const {
	return "GeometryCollection";
}

bool GeometryCollection::isSimple() const {
	throw new IllegalArgumentException("This method is not supported by GeometryCollection objects\n");
	return false;
}

Geometry* GeometryCollection::getBoundary() const {
	throw new IllegalArgumentException("This method is not supported by GeometryCollection objects\n");
	return NULL;
}

bool
GeometryCollection::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}
	const GeometryCollection* otherCollection=dynamic_cast<const GeometryCollection *>(other);
	if (geometries->size()!=otherCollection->geometries->size()) {
		return false;
	}
	for (unsigned int i=0; i<geometries->size(); i++) {
		//if (typeid(*((*geometries)[i]))!=typeid(Geometry)) {
		//	return false;
		//}
		//if (typeid(*((*(otherCollection->geometries))[i]))!=typeid(Geometry)) {
		//	return false;
		//}
		if (!((*geometries)[i]->equalsExact((*(otherCollection->geometries))[i],tolerance))) {
			return false;
		}
	}
	return true;
}

void GeometryCollection::apply_rw(CoordinateFilter *filter) {
	for (unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->apply_rw(filter);
	}
}

void GeometryCollection::apply_ro(CoordinateFilter *filter) const {
	for (unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->apply_ro(filter);
	}
}

void GeometryCollection::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
	for(unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->apply_ro(filter);
	}
}

void GeometryCollection::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
	for(unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->apply_rw(filter);
	}
}

void GeometryCollection::normalize() {
	for (unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->normalize();
	}
	sort(geometries->begin(),geometries->end(),greaterThen);
}

Envelope* GeometryCollection::computeEnvelopeInternal() const {
	Envelope* envelope=new Envelope();
	for (unsigned int i=0; i<geometries->size(); i++) {
		//Envelope *env=new Envelope(*((*geometries)[i]->getEnvelopeInternal()));
		const Envelope *env=(*geometries)[i]->getEnvelopeInternal();
		envelope->expandToInclude(env);
	}
	return envelope;
}

int GeometryCollection::compareToSameClass(const Geometry *gc) const {
	return compare(*geometries, *(((GeometryCollection*)gc)->geometries));
}

const Coordinate* GeometryCollection::getCoordinate() const
{
	// should use auto_ptr here or return NULL or throw an exception !
	// 	--strk;
	if (isEmpty()) return new Coordinate();
    	return (*geometries)[0]->getCoordinate();
}

/**
* @return the area of this collection
*/
double GeometryCollection::getArea() const {
	double area=0.0;
	for(unsigned int i=0;i<geometries->size();i++) {
//		area+=geometries.at(i)->getArea();
        area+=(*geometries)[i]->getArea();
	}
	return area;
}

/**
* @return the total length of this collection
*/
double GeometryCollection::getLength() const {
	double sum=0.0;
	for(unsigned int i=0;i<geometries->size();i++) {
        	sum+=(*geometries)[i]->getLength();
	}
	return sum;
}

void GeometryCollection::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
	for(unsigned int i=0;i<geometries->size();i++) {
        (*geometries)[i]->apply_rw(filter);
	}
}

void GeometryCollection::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
	for(unsigned int i=0;i<geometries->size();i++) {
        (*geometries)[i]->apply_ro(filter);
	}
}

GeometryCollection::~GeometryCollection(){
	for(int i=0;i<(int)geometries->size();i++) {
		delete (*geometries)[i];
	}
	delete geometries;
}

GeometryTypeId
GeometryCollection::getGeometryTypeId() const {
	return GEOS_GEOMETRYCOLLECTION;
}

} // namespace geos

/**********************************************************************
 * $Log$
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
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 **********************************************************************/

