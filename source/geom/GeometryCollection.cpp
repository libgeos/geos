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

/*
* Copies given vector<Geometry *> to private area.
* WARNING: Geometries pointed to by vector elements will
* be deleted by GeometryCollection destructor, so callers
* should not touch them anymore (althought they can safely
* delete vector used to store them).
* vector<Geometry *> const *newGeometries
* should be the correct form, but I'm coding too much ;0 --strk;
*/
GeometryCollection::GeometryCollection(vector<Geometry *> *newGeometries,
		PrecisionModel* pm,int SRID): Geometry(pm,SRID){
	if (newGeometries==NULL) {
		geometries=new vector<Geometry *>();
		return;
	}
	if (hasNullElements(newGeometries)) {
		throw new IllegalArgumentException("geometries must not contain null elements\n");
		return;
	}
	geometries=new vector<Geometry *>(*newGeometries);
}

CoordinateList* GeometryCollection::getCoordinates() const {
	CoordinateList *coordinates=CoordinateListFactory::internalFactory->createCoordinateList(getNumPoints());
	int k = -1;
	for (unsigned int i=0; i<geometries->size(); i++) {
		CoordinateList* childCoordinates=(*geometries)[i]->getCoordinates();
		for (int j=0; j<childCoordinates->getSize(); j++) {
			k++;
			coordinates->setAt(childCoordinates->getAt(j),k);
		}
		delete childCoordinates; // xie
	}
	return coordinates;
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
	Assert::shouldNeverReachHere();
    return false;
  }

Geometry* GeometryCollection::getBoundary() const {
	checkNotGeometryCollection(this);
	Assert::shouldNeverReachHere();
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
Point* GeometryCollection::getCentroid() const {
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
}

