#include "../headers/geom.h"
#include "../headers/util.h"
#include "../headers/geosAlgorithm.h"
#include <algorithm>
#include <typeinfo>

namespace geos {

GeometryCollection::GeometryCollection(){
	geometries=new vector<Geometry *>();
}

GeometryCollection::GeometryCollection(const GeometryCollection &gc):
	Geometry(gc.precisionModel,gc.SRID){
	geometries=gc.geometries;	
}

GeometryCollection::GeometryCollection(vector<Geometry *> *newGeometries,PrecisionModel* pm,int SRID):
	Geometry(pm,SRID){
	if (newGeometries==NULL) {
		newGeometries=new vector<Geometry *>();
	}
	if (hasNullElements(newGeometries)) {
		throw new IllegalArgumentException("geometries must not contain null elements\n");
	}
	geometries=newGeometries;
}

CoordinateList* GeometryCollection::getCoordinates() {
	CoordinateList *coordinates=CoordinateListFactory::internalFactory->createCoordinateList(getNumPoints());
	int k = -1;
	for (unsigned int i=0; i<geometries->size(); i++) {
	CoordinateList* childCoordinates=(*geometries)[i]->getCoordinates();
		for (int j=0; j<childCoordinates->getSize(); j++) {
			k++;
			coordinates->setAt(childCoordinates->getAt(j),k);
		}
	}
	return coordinates;
}

bool GeometryCollection::isEmpty() {
	for (unsigned int i=0; i<geometries->size(); i++) {
		if (!(*geometries)[i]->isEmpty()) {
			return false;
		}
	}
	return true;
}

int GeometryCollection::getDimension() {
	int dimension=Dimension::False;
	for (unsigned int i=0; i<geometries->size(); i++) {
		dimension=max(dimension,(*geometries)[i]->getDimension());
	}
	return dimension;
}

int GeometryCollection::getBoundaryDimension() {
	int dimension=Dimension::False;
	for(unsigned int i=0; i<geometries->size(); i++) {
		dimension=max(dimension,(*geometries)[i]->getBoundaryDimension());
	}
	return dimension;
}

int GeometryCollection::getNumGeometries() {
	return (int)geometries->size();
}

Geometry* GeometryCollection::getGeometryN(int n) {
	return (*geometries)[n];
}

int GeometryCollection::getNumPoints() {
	int numPoints = 0;
	for (unsigned int i=0; i<geometries->size(); i++) {
		numPoints +=(*geometries)[i]->getNumPoints();
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

Geometry* GeometryCollection::getBoundary() {
	checkNotGeometryCollection(this);
	Assert::shouldNeverReachHere();
	return NULL;
}

bool GeometryCollection::equalsExact(Geometry *other, double tolerance) {
	if (!isEquivalentClass(other)) {
		return false;
	}
	GeometryCollection* otherCollection=dynamic_cast<GeometryCollection *>(other);
	if (geometries->size()!=otherCollection->geometries->size()) {
		return false;
	}
	for (unsigned int i=0; i<geometries->size(); i++) {
		if (typeid(*((*geometries)[i]))!=typeid(Geometry)) {
			return false;
		}
		if (typeid(*((*(otherCollection->geometries))[i]))!=typeid(Geometry)) {
			return false;
		}
		if (!((*geometries)[i]->equalsExact((*(otherCollection->geometries))[i],tolerance))) {
			return false;
		}
	}
	return true;
}

void GeometryCollection::apply(CoordinateFilter *filter) {
	for (unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->apply(filter);
	}
}

void GeometryCollection::apply(GeometryFilter *filter) {
	filter->filter(this);
	for(unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->apply(filter);
	}
}

void GeometryCollection::normalize() {
	for (unsigned int i=0; i<geometries->size(); i++) {
		(*geometries)[i]->normalize();
	}
	sort(geometries->begin(),geometries->end(),greaterThen);
}

Envelope* GeometryCollection::computeEnvelopeInternal() {
	Envelope* envelope=new Envelope();
	for (unsigned int i=0; i<geometries->size(); i++) {
		envelope->expandToInclude((*geometries)[i]->getEnvelopeInternal());
	}
	return envelope;
}

int GeometryCollection::compareToSameClass(Geometry *gc) {
	return compare(*geometries, *(((GeometryCollection*)gc)->geometries));
}

Coordinate* GeometryCollection::getCoordinate() {
	if (isEmpty()) return new Coordinate();
    return (*geometries)[0]->getCoordinate();
}

/**
*  Returns the area of this <code>GeometryCollection</code>
*
*@return the area of the polygon
*/
double GeometryCollection::getArea() {
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
double GeometryCollection::getLength() {
	double sum=0.0;
	for(unsigned int i=0;i<geometries->size();i++) {
        sum+=(*geometries)[i]->getLength();
	}
	return sum;
}

void GeometryCollection::apply(GeometryComponentFilter *filter) {
	filter->filter(this);
	for(unsigned int i=0;i<geometries->size();i++) {
        (*geometries)[i]->apply(filter);
	}
}

GeometryCollection::~GeometryCollection(void){
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
Point* GeometryCollection::getCentroid() {
	Coordinate* centPt;
	int dim=getDimension();
	if(dim==0) {
		CentroidPoint *cent=new CentroidPoint();
		cent->add(this);
		centPt=cent->getCentroid();
	} else if (dim==1) {
		CentroidLine *cent=new CentroidLine();
		cent->add(this);
		centPt=cent->getCentroid();
	} else {
		CentroidArea *cent=new CentroidArea();
		cent->add(this);
		centPt=cent->getCentroid();
	}
	return GeometryFactory::createPointFromInternalCoord(centPt,this);
}
}