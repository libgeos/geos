#include "geom.h"
#include "util.h"
#include <algorithm>
#include <typeinfo>

GeometryCollection::GeometryCollection(){}

GeometryCollection::GeometryCollection(const GeometryCollection &gc):
	Geometry(gc.precisionModel,gc.SRID), geometries(gc.geometries) {}

GeometryCollection::GeometryCollection(vector<Geometry *> *newGeometries,PrecisionModel pm,int SRID):
	Geometry(pm,SRID){
	if (newGeometries==NULL) {
		newGeometries=new vector<Geometry *>();
	}
	if (hasNullElements(geometries)) {
		throw "IllegalArgumentException: geometries must not contain null elements\n";
	}
	geometries=*newGeometries;
}

CoordinateList GeometryCollection::getCoordinates() {
	CoordinateList coordinates(getNumPoints());
	int k = -1;
	for (unsigned int i=0; i<geometries.size(); i++) {
	CoordinateList childCoordinates(geometries[i]->getCoordinates());
		for (int j=0; j<childCoordinates.getSize(); j++) {
			k++;
			coordinates.setAt(childCoordinates.getAt(j),k);
		}
	}
	return coordinates;
}

bool GeometryCollection::isEmpty() {
	for (unsigned int i=0; i<geometries.size(); i++) {
		if (!geometries[i]->isEmpty()) {
			return false;
		}
	}
	return true;
}

int GeometryCollection::getDimension() {
	int dimension=Dimension::False;
	for (unsigned int i=0; i<geometries.size(); i++) {
		dimension=max(dimension,geometries[i]->getDimension());
	}
	return dimension;
}

int GeometryCollection::getBoundaryDimension() {
	int dimension=Dimension::False;
	for(unsigned int i=0; i<geometries.size(); i++) {
		dimension=max(dimension,geometries[i]->getBoundaryDimension());
	}
	return dimension;
}

int GeometryCollection::getNumGeometries() {
	return (int)geometries.size();
}

Geometry *GeometryCollection::getGeometryN(int n) {
	return geometries[n];
}

int GeometryCollection::getNumPoints() {
	int numPoints = 0;
	for (unsigned int i=0; i<geometries.size(); i++) {
		numPoints +=geometries[i]->getNumPoints();
	}
	return numPoints;
}

string GeometryCollection::getGeometryType() {
	return "GeometryCollection";
}

bool GeometryCollection::isSimple() {
	checkNotGeometryCollection(this);
	Assert::shouldNeverReachHere();
    return false;
  }

Geometry GeometryCollection::getBoundary() {
	checkNotGeometryCollection(this);
	Assert::shouldNeverReachHere();
	return Geometry();
}

bool GeometryCollection::equalsExact(Geometry *other) {
	if (!isEquivalentClass(other)) {
		return false;
	}
	GeometryCollection otherCollection(*dynamic_cast<GeometryCollection *>(other));
	if (geometries.size()!=otherCollection.geometries.size()) {
		return false;
	}
	for (unsigned int i=0; i<geometries.size(); i++) {
		if (typeid(*(geometries[i]))!=typeid(Geometry)) {
			return false;
		}
		if (typeid(*(otherCollection.geometries[i]))!=typeid(Geometry)) {
			return false;
		}
		if (!(geometries[i]->equalsExact(otherCollection.geometries[i]))) {
			return false;
		}
	}
	return true;
}

void GeometryCollection::apply(CoordinateFilter *filter) {
	for (unsigned int i=0; i<geometries.size(); i++) {
		geometries[i]->apply(filter);
	}
}

void GeometryCollection::apply(GeometryFilter *filter) {
	filter->filter(this);
	for(unsigned int i=0; i<geometries.size(); i++) {
		geometries[i]->apply(filter);
	}
}

void GeometryCollection::normalize() {
	for (unsigned int i=0; i<geometries.size(); i++) {
		geometries[i]->normalize();
	}
	sort(geometries.begin(),geometries.end(),greaterThen);
}

Envelope GeometryCollection::computeEnvelopeInternal() {
	Envelope envelope;
	for (unsigned int i=0; i<geometries.size(); i++) {
		envelope.expandToInclude(geometries[i]->getEnvelopeInternal());
	}
	return envelope;
}

int GeometryCollection::compareToSameClass(GeometryCollection *gc) {
	return compare(geometries, gc->geometries);
}

Coordinate GeometryCollection::getCoordinate() {
	if (isEmpty()) return Coordinate();
//	return geometries.at(0)->getCoordinate();
    return geometries[0]->getCoordinate();
}

/**
*  Returns the area of this <code>GeometryCollection</code>
*
*@return the area of the polygon
*/
double GeometryCollection::getArea() {
	double area=0.0;
	for(unsigned int i=0;i<geometries.size();i++) {
//		area+=geometries.at(i)->getArea();
        area+=geometries[i]->getArea();
	}
	return area;
}

/**
*  Returns the area of this <code>MultiLineString</code>
*
*@return the area of the polygon
*/
double GeometryCollection::getLength() {
	double sum=0.0;
	for(unsigned int i=0;i<geometries.size();i++) {
//		sum+=((LineString*)geometries.at(i))->getLength();
        sum+=((LineString*)geometries[i])->getLength();
	}
	return sum;
}

void GeometryCollection::apply(GeometryComponentFilter *filter) {
	filter->filter(this);
	for(unsigned int i=0;i<geometries.size();i++) {
//		geometries.at(i)->apply(filter);
        geometries[i]->apply(filter);
	}
}

GeometryCollection::~GeometryCollection(void){}
