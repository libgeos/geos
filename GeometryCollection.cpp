#include "geom.h"
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

Geometry GeometryCollection::getGeometryN(int n) {
	return *geometries[n];
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

//!!! External Dependency
bool GeometryCollection::isSimple() {
	checkNotGeometryCollection(this);
//!!! External Dependency
//	Assert.shouldNeverReachHere();
    return false;
  }

//!!! External Dependency
Geometry GeometryCollection::getBoundary() {
	checkNotGeometryCollection(this);
//!!! External Dependency
//	Assert.shouldNeverReachHere();
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

GeometryCollection::~GeometryCollection(void){}
