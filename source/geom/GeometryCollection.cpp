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
 * Revision 1.35  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.34  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.33  2004/06/15 20:07:51  strk
 * GeometryCollections constructors make a deep copy of Geometry vector argument.
 *
 * Revision 1.32  2004/05/17 21:14:47  ybychkov
 * JavaDoc updated
 *
 * Revision 1.31  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateSequence.
 *
 * Revision 1.30  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.29  2004/04/14 12:28:43  strk
 * shouldNeverReachHere exceptions made more verbose
 *
 * Revision 1.28  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.27  2003/12/11 15:53:40  strk
 * Fixed bogus copy constructor (making clone bogus)
 *
 * Revision 1.26  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geom.h>
#include <geos/util.h>
#include <geos/geosAlgorithm.h>
#include <algorithm>
#include <typeinfo>

namespace geos {

//GeometryCollection::GeometryCollection(){
//	geometries=new vector<Geometry *>();
//}

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

/**
* Collects all coordinates of all subgeometries into a CoordinateSequence.
* 
* Note that while changes to the coordinate objects themselves
* may modify the Geometries in place, the returned CoordinateSequence as such 
* is only a temporary container which is not synchronized back.
* 
* @return the collected coordinates
*
*/
CoordinateSequence* GeometryCollection::getCoordinates() const {
	vector<Coordinate> *coordinates = new vector<Coordinate>(getNumPoints());

	int k = -1;
	for (unsigned int i=0; i<geometries->size(); i++) {
		CoordinateSequence* childCoordinates=(*geometries)[i]->getCoordinates();
		for (int j=0; j<childCoordinates->getSize(); j++) {
			k++;
			(*coordinates)[k] = childCoordinates->getAt(j);
		}
		delete childCoordinates; // xie
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
	checkNotGeometryCollection(this);
	Assert::shouldNeverReachHere("isSimple() on GeometryCollection failed to throw an exception");
    return false;
  }

Geometry* GeometryCollection::getBoundary() const {
	checkNotGeometryCollection(this);
	Assert::shouldNeverReachHere("getBoundary() on GeometryCollection failed to throw an exception");
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
		Envelope *env=(*geometries)[i]->getEnvelopeInternal();
		envelope->expandToInclude(env);
		delete env;
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
*  Returns the area of this <code>GeometryCollection</code>
*
*@return the area of the polygon
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
*  Returns the area of this <code>MultiLineString</code>
*
*@return the area of the polygon
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

/**
* Computes the centroid of a heterogenous GeometryCollection.
* The centroid
* is equal to the centroid of the set of component Geometrys of highest
* dimension (since the lower-dimension geometries contribute zero
* "weight" to the centroid)
* @return
*/
//Point* GeometryCollection::getCentroid() const {
//	Coordinate* centPt;
//	int dim=getDimension();
//	if(dim==0) {
//		CentroidPoint *cent=new CentroidPoint();
//		cent->add(this);
//		centPt=cent->getCentroid();
//		delete cent;
//	} else if (dim==1) {
//		CentroidLine *cent=new CentroidLine();
//		cent->add(this);
//		centPt=cent->getCentroid();
//		delete cent;
//	} else {
//		CentroidArea *cent=new CentroidArea();
//		cent->add(this);
//		centPt=cent->getCentroid();
//		delete cent;
//	}
//	Point *pt=GeometryFactory::createPointFromInternalCoord(centPt,this);
//	delete centPt;
//	return pt;
//}

GeometryTypeId
GeometryCollection::getGeometryTypeId() const {
	return GEOS_GEOMETRYCOLLECTION;
}
}

