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
 * Revision 1.38  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.37  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.36  2003/10/20 15:41:34  strk
 * Geometry::checkNotGeometryCollection made static and non-distructive.
 *
 * Revision 1.35  2003/10/13 12:51:28  strk
 * removed sortedClasses strings array from all geometries.
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/util.h"
#include <typeinfo>
#include <algorithm>
#include "../headers/geosAlgorithm.h"
#include "../headers/operation.h"
#include "../headers/opRelate.h"
#include "../headers/opValid.h"
#include "../headers/opDistance.h"
#include "../headers/opOverlay.h"
#include "../headers/opBuffer.h"
#include "../headers/io.h"

namespace geos {

CGAlgorithms* Geometry::cgAlgorithms=new RobustCGAlgorithms();
GeometryComponentFilter* Geometry::geometryChangedFilter=new GeometryComponentFilter();

Geometry::Geometry() {
	SRID=0;
	precisionModel=new PrecisionModel();
	envelope=new Envelope();
}

Geometry::Geometry(const Geometry &geom) {
	precisionModel=new PrecisionModel(*geom.precisionModel);
	envelope=new Envelope(*(geom.envelope));
	SRID=geom.SRID;
}

Geometry::Geometry(const PrecisionModel *pm, int newSRID){
	precisionModel=new PrecisionModel(*pm);
	envelope=new Envelope();
	SRID = newSRID;
}

bool Geometry::hasNonEmptyElements(vector<Geometry *>* geometries) {
	for (unsigned int i=0; i<geometries->size(); i++) {
		if (!(*geometries)[i]->isEmpty()) {
			return true;
		}
	}
	return false;
}

bool Geometry::hasNullElements(const CoordinateList* list){
	for (int i = 0; i<list->getSize(); i++) {
		if (list->getAt(i)==Coordinate::getNull()) {
			return true;
		}
	}
	return false;
}

bool Geometry::hasNullElements(vector<Geometry *>* lrs) {
	for (unsigned int i = 0; i<lrs->size(); i++) {
		if ((*lrs)[i]==NULL) {
			return true;
		}
	}
	return false;
}
	
//void Geometry::reversePointOrder(CoordinateList* coordinates) {
//	int length=coordinates->getSize();
//	vector<Coordinate> v(length);
//	for (int i=0; i<length; i++) {
//		v[i]=coordinates->getAt(length - 1 - i);
//	}
//	coordinates->setPoints(v);
//}
	
//Coordinate& Geometry::minCoordinate(CoordinateList* coordinates){
//	vector<Coordinate> v(*(coordinates->toVector()));
//	sort(v.begin(),v.end(),lessThen);
//	return v.front();
//}

//void Geometry::scroll(CoordinateList* coordinates,Coordinate* firstCoordinate) {
//	int ind=indexOf(firstCoordinate,coordinates);
//	Assert::isTrue(ind > -1);
//	int length=coordinates->getSize();
//	vector<Coordinate> v(length);
//	for (int i=ind; i<length; i++) {
//		v[i-ind]=coordinates->getAt(i);
//	}
//	for (int j=0; j<ind; j++) {
//		v[length-ind+j]=coordinates->getAt(j);
//	}
//	coordinates->setPoints(v);
//}
//
//int Geometry::indexOf(Coordinate* coordinate,CoordinateList* coordinates) {
//	for (int i=0; i<coordinates->getSize(); i++) {
//		if ((*coordinate)==coordinates->getAt(i)) {
//			return i;
//		}
//	}
//	return -1;
//}

/**
* Tests whether the distance from this <code>Geometry</code>
* to another is less than or equal to a specified value.
*
* @param geom the Geometry to check the distance to
* @param distance the distance value to compare
* @return <code>true</code> if the geometries are less than <code>distance</code> apart.
*/
bool Geometry::isWithinDistance(Geometry *geom,double cDistance) {
	Envelope *env0=getEnvelopeInternal();
	Envelope *env1=geom->getEnvelopeInternal();
	double envDist=env0->distance(env1);
	delete env0;
	delete env1;
	if (envDist>cDistance)
		return false;
	// NOTE: this could be implemented more efficiently
	double geomDist=distance(geom);
	if (geomDist>cDistance)
		return false;
	return true;
}

/**
* Computes the centroid of this <code>Geometry</code>.
* The centroid
* is equal to the centroid of the set of component Geometrys of highest
* dimension (since the lower-dimension geometries contribute zero
* "weight" to the centroid)
*
* @return a {@link Point} which is the centroid of this Geometry
*/
Point* Geometry::getCentroid() const {
	Coordinate* centPt;
	int dim=getDimension();
	if(dim==0) {
		CentroidPoint *cent=new CentroidPoint();
		cent->add(this);
		centPt=cent->getCentroid();
		delete cent;
	} else if (dim==1) {
		CentroidLine *cent=new CentroidLine();
		cent->add(this);
		centPt=cent->getCentroid();
		delete cent;
	} else {
		CentroidArea *cent=new CentroidArea();
		cent->add(this);
		centPt=cent->getCentroid();
		delete cent;
	}
	Point *pt=GeometryFactory::createPointFromInternalCoord(centPt,this);
	delete centPt;
	return pt;
}

/**
* Computes an interior point of this <code>Geometry</code>.
* An interior point is guaranteed to lie in the interior of the Geometry,
* if it possible to calculate such a point exactly. Otherwise,
* the point may lie on the boundary of the geometry.
*
* @return a {@link Point} which is in the interior of this Geometry
*/
Point* Geometry::getInteriorPoint() {
	const Coordinate* interiorPt;
	int dim=getDimension();
	if (dim==0) {
		InteriorPointPoint* intPt=new InteriorPointPoint(this);
		interiorPt=intPt->getInteriorPoint();
		delete intPt;
	} else if (dim==1) {
		InteriorPointLine* intPt=new InteriorPointLine(this);
		interiorPt=intPt->getInteriorPoint();
		delete intPt;
	} else {
		InteriorPointArea* intPt=new InteriorPointArea(this);
		interiorPt=intPt->getInteriorPoint();
		delete intPt;
	}
	Point *p=GeometryFactory::createPointFromInternalCoord(interiorPt,this);;
	delete interiorPt;
	return p;
}

/**
* Notifies this Geometry that its Coordinates have been changed by an external
* party (using a CoordinateFilter, for example). The Geometry will flush
* and/or update any information it has cached (such as its {@link Envelope} ).
*/
void Geometry::geometryChanged() {
	apply_rw(geometryChangedFilter);
}

/**
* Notifies this Geometry that its Coordinates have been changed by an external
* party. When #geometryChanged is called, this method will be called for
* this Geometry and its component Geometries.
* @see #apply(GeometryComponentFilter)
*/
void Geometry::geometryChangedAction() {
	envelope=NULL;
}

int Geometry::getSRID() const {return SRID;}

void Geometry::setSRID(int newSRID) {SRID=newSRID;}

PrecisionModel* Geometry::getPrecisionModel() const {return precisionModel;}

bool Geometry::isValid() const {
	IsValidOp isValidOp(this);
	return isValidOp.isValid();
}

Geometry* Geometry::getEnvelope() const {
	return GeometryFactory::toGeometry(getEnvelopeInternal(),precisionModel,SRID);
}

Envelope* Geometry::getEnvelopeInternal() const {
	if (envelope->isNull()) {
		return computeEnvelopeInternal();
	} else 
		return new Envelope(*envelope);
}

bool Geometry::disjoint(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isDisjoint();
	delete im;
	return res;
}

bool Geometry::touches(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isTouches(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool Geometry::intersects(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isIntersects();
	delete im;
	return res;
}

bool Geometry::crosses(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isCrosses(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool Geometry::within(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isWithin();
	delete im;
	return res;
}

bool Geometry::contains(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isContains();
	delete im;
	return res;
}

bool Geometry::overlaps(const Geometry *g) const{
	IntersectionMatrix *im=relate(g);
	bool res=im->isOverlaps(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool Geometry::relate(const Geometry *g, string intersectionPattern) const {
	IntersectionMatrix *im=relate(g);
	bool res=im->matches(intersectionPattern);
	delete im;
	return res;
}

bool Geometry::equals(const Geometry *g) const {
	IntersectionMatrix *im=relate(g);
	bool res=im->isEquals(getDimension(), g->getDimension());
	delete im;
	return res;
}

IntersectionMatrix* Geometry::relate(const Geometry *g) const {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(g);
	return RelateOp::relate(this,g);
}

string Geometry::toString() const {
	return toText();
}

string Geometry::toText() const {
	WKTWriter writer;
	return writer.write(this);
}

Geometry* Geometry::buffer(double distance) const {
	return BufferOp::bufferOp(this, distance);
}

/**
*  Returns a buffer region around this <code>Geometry</code> having the given
*  width and with a specified number of segments used to approximate curves.
* The buffer of a Geometry is the Minkowski sum of the Geometry with
* a disc of radius <code>distance</code>.  Curves in the buffer polygon are
* approximated with line segments.  This method allows specifying the
* accuracy of that approximation.
*
*@param  distance  the width of the buffer, interpreted according to the
*      <code>PrecisionModel</code> of the <code>Geometry</code>
*@param quadrantSegments the number of segments to use to approximate a quadrant of a circle
*@return           all points whose distance from this <code>Geometry</code>
*      are less than or equal to <code>distance</code>
*/
Geometry* Geometry::buffer(double distance,int quadrantSegments) const {
	return BufferOp::bufferOp(this, distance, quadrantSegments);
}

Geometry* Geometry::convexHull() const {
	ConvexHull *ch=new ConvexHull(cgAlgorithms);
	Geometry *g=ch->getConvexHull(this);
	delete ch;
	return g;
}

Geometry* Geometry::intersection(const Geometry *other) const {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	return OverlayOp::overlayOp(this,other,OverlayOp::INTERSECTION);
}

Geometry* Geometry::Union(const Geometry *other) const {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	return OverlayOp::overlayOp(this,other,OverlayOp::UNION);
}

Geometry* Geometry::difference(const Geometry *other) const {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	return OverlayOp::overlayOp(this,other,OverlayOp::DIFFERENCE);
}

Geometry* Geometry::symDifference(const Geometry *other) const {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	return OverlayOp::overlayOp(this,other,OverlayOp::SYMDIFFERENCE);
}

int Geometry::compareTo(const Geometry *geom) const {
	if (getClassSortIndex()!=geom->getClassSortIndex()) {
		return getClassSortIndex()-geom->getClassSortIndex();
	}
	if (isEmpty() && geom->isEmpty()) {
		return 0;
	}
	if (isEmpty()) {
		return -1;
	}
	if (geom->isEmpty()) {
		return 1;
	}
	return compareToSameClass(geom);
}

bool Geometry::isEquivalentClass(const Geometry *other) const {
	if (typeid(*this)==typeid(*other))
		return true;
	else
		return false;
}

void Geometry::checkNotGeometryCollection(const Geometry *g) {
	if ((typeid(*g)==typeid(GeometryCollection))) {
		throw new IllegalArgumentException("This method does not support GeometryCollection arguments\n");
	}
}

//void Geometry::checkEqualSRID(Geometry *other) {
//	if (SRID!=other->getSRID()) {
//		throw "IllegalArgumentException: Expected SRIDs to be equal, but they were not\n";
//	}
//}
//
//void Geometry::checkEqualPrecisionModel(Geometry *other) {
//	if (!((*precisionModel)==(*(other->getPrecisionModel())))) {
//		throw "IllegalArgumentException: Expected precision models to be equal, but they were not\n";
//	}
//}

int Geometry::getClassSortIndex() const {
	//const type_info &t=typeid(*this);

	     if ( typeid(*this) == typeid(Point)              ) return 0;
	else if ( typeid(*this) == typeid(MultiPoint)         ) return 1;
	else if ( typeid(*this) == typeid(LineString)         ) return 2;
	else if ( typeid(*this) == typeid(LinearRing)         ) return 3;
	else if ( typeid(*this) == typeid(MultiLineString)    ) return 4;
	else if ( typeid(*this) == typeid(Polygon)            ) return 5;
	else if ( typeid(*this) == typeid(MultiPolygon)       ) return 6;
	else if ( typeid(*this) == typeid(GeometryCollection) ) return 7;

	string str="Class not supported: ";
	str.append(typeid(*this).name());
	str.append("");
	Assert::shouldNeverReachHere(str);
	return -1;
}

int Geometry::compare(vector<Coordinate> a, vector<Coordinate> b) const {
	unsigned int i=0;
	unsigned int j=0;
	while (i<a.size() && j<b.size()) {
		Coordinate& aCoord=a[i];
		Coordinate& bCoord=b[j];
		int comparison=aCoord.compareTo(bCoord);
		if (comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<a.size()) {
		return 1;
	}
	if (j<b.size()) {
		return -1;
	}
	return 0;
}

int Geometry::compare(vector<Geometry *> a, vector<Geometry *> b) const {
	unsigned int i=0;
	unsigned int j=0;
	while (i<a.size() && j<b.size()) {
		Geometry *aGeom=a[i];
		Geometry *bGeom=b[j];
		int comparison=aGeom->compareTo(bGeom);
		if (comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<a.size()) {
		return 1;
	}
	if (j<b.size()) {
		return -1;
	}
	return 0;
}

/**
*  Returns the minimum distance between this <code>Geometry</code>
*  and the <code>Geometry</code> g
*
*@param  g  the <code>Geometry</code> from which to compute the distance
*/
double Geometry::distance(const Geometry *g) const {
	return DistanceOp::distance(this,g);
}

/**
*  Returns the area of this <code>Geometry</code>.
*  Areal Geometries have a non-zero area.
*  They override this function to compute the area.
*  Others return 0.0
*
*@return the area of the Geometry
*/
double Geometry::getArea() const {
	return 0.0;
}

/**
*  Returns the length of this <code>Geometry</code>.
*  Linear geometries return their length.
*  Areal geometries return their perimeter.
*  They override this function to compute the area.
*  Others return 0.0
*
*@return the length of the Geometry
*/
double Geometry::getLength() const {
	return 0.0;
}


Geometry::~Geometry(){
	delete precisionModel;
	delete envelope;
}

bool lessThen(Coordinate& a, Coordinate& b) {
	if (a.compareTo(b)<=0)
		return true;
	else
		return false;
}

bool greaterThen(Geometry *first, Geometry *second) {
	if (first->compareTo(second)>=0)
		return true;
	else
		return false;
}

bool
Geometry::equal(const Coordinate& a, const Coordinate& b,double tolerance) const
{
	if (tolerance==0) {return a==b;}
	return a.distance(b)<=tolerance;
}

void Geometry::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

void Geometry::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Geometry::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

void Geometry::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}

}

