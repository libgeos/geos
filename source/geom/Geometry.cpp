#include "geom.h"
#include "util.h"
#include <typeinfo>
#include <algorithm>
#include "geosAlgorithm.h"
#include "operation.h"
#include "opRelate.h"
#include "io.h"


CGAlgorithms* Geometry::cgAlgorithms=new RobustCGAlgorithms();
Geometry::Geometry(): precisionModel(PrecisionModel()), envelope(Envelope()){
	SRID=0;
	sortedClasses.push_back(typeid(Point).name());
	sortedClasses.push_back(typeid(MultiPoint).name());
	sortedClasses.push_back(typeid(LineString).name());
	sortedClasses.push_back(typeid(LinearRing).name());
	sortedClasses.push_back(typeid(MultiLineString).name());
	sortedClasses.push_back(typeid(Polygon).name());
	sortedClasses.push_back(typeid(MultiPolygon).name());
	sortedClasses.push_back(typeid(GeometryCollection).name());
}

Geometry::Geometry(const Geometry &geom): precisionModel(geom.precisionModel), envelope(geom.envelope),
sortedClasses(geom.sortedClasses) {
	SRID=geom.SRID;
}

Geometry::Geometry(PrecisionModel newPrecisionModel, int newSRID):
	precisionModel(newPrecisionModel), envelope(Envelope()) {
	SRID = newSRID;
	sortedClasses.push_back(typeid(Point).name());
	sortedClasses.push_back(typeid(MultiPoint).name());
	sortedClasses.push_back(typeid(LineString).name());
	sortedClasses.push_back(typeid(LinearRing).name());
	sortedClasses.push_back(typeid(MultiLineString).name());
	sortedClasses.push_back(typeid(Polygon).name());
	sortedClasses.push_back(typeid(MultiPolygon).name());
	sortedClasses.push_back(typeid(GeometryCollection).name());
}

bool Geometry::hasNonEmptyElements(vector<Geometry *> geometries) {
	for (unsigned int i=0; i<geometries.size(); i++) {
		if (!geometries[i]->isEmpty()) {
			return true;
		}
	}
	return false;
}

bool Geometry::hasNullElements(CoordinateList* list){
	for (int i = 0; i<list->getSize(); i++) {
		if (list->getAt(i)==Coordinate::getNull()) {
			return true;
		}
	}
	return false;
}

bool Geometry::hasNullElements(vector<Geometry *> lrs) {
	for (unsigned int i = 0; i<lrs.size(); i++) {
		if (lrs[i]==NULL) {
			return true;
		}
	}
	return false;
}
	
void Geometry::reversePointOrder(CoordinateList* coordinates) {
	int length=coordinates->getSize();
	vector<Coordinate> v(length);
	for (int i=0; i<length; i++) {
		v[i]=coordinates->getAt(length - 1 - i);
	}
	coordinates->setPoints(v);
}
	
Coordinate& Geometry::minCoordinate(CoordinateList* coordinates){
	vector<Coordinate> v(*(coordinates->toVector()));
	sort(v.begin(),v.end(),lessThen);
	return v.front();
}

void Geometry::scroll(CoordinateList* coordinates,Coordinate& firstCoordinate) {
	int ind=indexOf(firstCoordinate,coordinates);
	Assert::isTrue(ind > -1);
	int length=coordinates->getSize();
	vector<Coordinate> v(length);
	for (int i=ind; i<length; i++) {
		v[i-ind]=coordinates->getAt(i);
	}
	for (int j=0; j<ind; j++) {
		v[length-ind+j]=coordinates->getAt(j);
	}
	coordinates->setPoints(v);
}

int Geometry::indexOf(Coordinate& coordinate,CoordinateList* coordinates) {
	for (int i=0; i<coordinates->getSize(); i++) {
		if (coordinate==coordinates->getAt(i)) {
			return i;
		}
	}
	return -1;
}

int Geometry::getSRID() {return SRID;}

void Geometry::setSRID(int newSRID) {SRID=newSRID;}

PrecisionModel Geometry::getPrecisionModel() {return precisionModel;}

//!!! External Dependency
bool Geometry::isValid() {
//	IsValidOp isValidOp(this);
//	return isValidOp.isValid();
	return false;
}

Geometry Geometry::getEnvelope() {
	return GeometryFactory::toGeometry(getEnvelopeInternal(),precisionModel,SRID);
}

Envelope Geometry::getEnvelopeInternal() {
	if (envelope.isNull()) {
		return computeEnvelopeInternal();
	} else 
		return envelope;
}

bool Geometry::disjoint(Geometry *g){
	return relate(g).isDisjoint();
}

bool Geometry::touches(Geometry *g){
	return relate(g).isTouches(getDimension(), g->getDimension());
}

bool Geometry::intersects(Geometry *g){
	return relate(g).isIntersects();
}

bool Geometry::crosses(Geometry *g){
	return relate(g).isCrosses(getDimension(), g->getDimension());
}

bool Geometry::within(Geometry *g){
	return relate(g).isWithin();
}

bool Geometry::contains(Geometry *g){
	return relate(g).isContains();
}

bool Geometry::overlaps(Geometry *g){
	return relate(g).isOverlaps(getDimension(), g->getDimension());
}

bool Geometry::relate(Geometry *g, string intersectionPattern) {
	return relate(g).matches(intersectionPattern);
}

bool Geometry::equals(Geometry *g){
	return relate(g).isEquals(getDimension(), g->getDimension());
}

IntersectionMatrix Geometry::relate(Geometry *g) {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(g);
	checkEqualSRID(g);
	checkEqualPrecisionModel(g);
	return RelateOp::relate(this,g);
}

string Geometry::toString() {
	return toText();
}

string Geometry::toText() {
	WKTWriter writer;
	return writer.write(this);
}

//!!! External Dependency
Geometry Geometry::buffer(double distance) {
//!!! External Dependency
//	return BufferOp.bufferOp(this, distance);
	return Geometry();
}

//!!! External Dependency
Geometry Geometry::convexHull() {
//!!! External Dependency
//	return (new ConvexHull(cgAlgorithms)).getConvexHull(this);
	return Geometry();
}

//!!! External Dependency
Geometry Geometry::intersection(Geometry *other) {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	checkEqualSRID(other);
	checkEqualPrecisionModel(other);
//!!! External Dependency
	//return OverlayOp.overlayOp(this, other, OverlayOp::INTERSECTION);
	return Geometry();
}

//!!! External Dependency
Geometry Geometry::Union(Geometry *other) {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	checkEqualSRID(other);
	checkEqualPrecisionModel(other);
//!!! External Dependency
	//return OverlayOp.overlayOp(this, other, OverlayOp::UNION);
	return Geometry();
}

//!!! External Dependency
Geometry Geometry::difference(Geometry *other) {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	checkEqualSRID(other);
	checkEqualPrecisionModel(other);
//!!! External Dependency
	//return OverlayOp.overlayOp(this, other, OverlayOp::DIFFERENCE);
	return Geometry();
}

//!!! External Dependency
Geometry Geometry::symDifference(Geometry *other) {
	checkNotGeometryCollection(this);
	checkNotGeometryCollection(other);
	checkEqualSRID(other);
	checkEqualPrecisionModel(other);
//!!! External Dependency
	//return OverlayOp.overlayOp(this, other, OverlayOp::SYMDIFFERENCE);
	return Geometry();
}

int Geometry::compareTo(Geometry *geom){
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

bool Geometry::isEquivalentClass(Geometry *other){
	if (typeid(*this)==typeid(*other))
		return true;
	else
		return false;
}

void Geometry::checkNotGeometryCollection(Geometry *g){
	if ((typeid(*g)==typeid(GeometryCollection)))
		throw "IllegalArgumentException: This method does not support GeometryCollection arguments\n";
}

void Geometry::checkEqualSRID(Geometry *other) {
	if (SRID!=other->getSRID()) {
		throw "IllegalArgumentException: Expected SRIDs to be equal, but they were not\n";
	}
}

void Geometry::checkEqualPrecisionModel(Geometry *other) {
	if (!(precisionModel==other->getPrecisionModel())) {
		throw "IllegalArgumentException: Expected precision models to be equal, but they were not\n";
	}
}

int Geometry::getClassSortIndex() {
    const type_info &t=typeid(*this);
    string tst=t.name();
	for (unsigned int i=0; i<sortedClasses.size(); i++) {
//		string tst2=sortedClasses[i];
		if ( sortedClasses[i]==typeid(*this).name() ) {
			return i;
		}
	}
	string str="Class not supported: ";
	str.append(typeid(*this).name());
	str.append("");
	Assert::shouldNeverReachHere(str);
	return -1;
}

int Geometry::compare(vector<Coordinate> a, vector<Coordinate> b){
	unsigned int i=0;
	unsigned int j=0;
	while (i<a.size() && j<b.size()) {
		Coordinate aCoord=a[i];
		Coordinate bCoord=b[j];
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

int Geometry::compare(vector<Geometry *> a, vector<Geometry *> b) {
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
double Geometry::distance(Geometry *g) {
//!!! External Dependency
//	return DistanceOp::distance(this,g);
	return 0.0;
}

/**
*  Returns the area of this <code>Geometry</code>.
*  Areal Geometries have a non-zero area.
*  They override this function to compute the area.
*  Others return 0.0
*
*@return the area of the Geometry
*/
double Geometry::getArea() {
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
double Geometry::getLength() {
	return 0.0;
}


Geometry::~Geometry(){}

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
