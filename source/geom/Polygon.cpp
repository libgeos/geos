#include "geom.h"
#include <typeinfo>
#include "geosAlgorithm.h"

Polygon::Polygon(){}
Polygon::Polygon(const Polygon &p): Geometry(p.precisionModel, p.SRID), holes(p.holes) {
//	LinearRing s(p.shell);
//	vector<Geometry *> h(p.holes.begin(),p.holes.end());
	shell=p.shell;
//	holes=p.holes;
}

Polygon::Polygon(LinearRing *newShell, PrecisionModel* precisionModel, int SRID){
	Polygon(newShell, &vector<Geometry *>(), precisionModel, SRID);
}

Polygon::Polygon(LinearRing *newShell, vector<Geometry *> *newHoles,
				 PrecisionModel* precisionModel, int SRID):
				Geometry(precisionModel, SRID) {
	if (newShell==NULL)
		newShell=new LinearRing(CoordinateListFactory::internalFactory->createCoordinateList(), precisionModel, SRID);

	if (newHoles==NULL)
		newHoles=new vector<Geometry *>();

	if (hasNullElements(*newHoles)) {
		throw "IllegalArgumentException: holes must not contain null elements";
	}
	if (newShell->isEmpty() && hasNonEmptyElements(*newHoles)) {
		throw "IllegalArgumentException: shell is empty but holes are not";
	}
	shell=newShell;
	holes=*newHoles;
}

CoordinateList* Polygon::getCoordinates() {
	if (isEmpty()) {
		return CoordinateListFactory::internalFactory->createCoordinateList();
	}
	CoordinateList *coordinates=CoordinateListFactory::internalFactory->createCoordinateList(getNumPoints());
	int k = -1;
	CoordinateList* shellCoordinates=shell->getCoordinates();
	for (int x = 0; x < shellCoordinates->getSize(); x++) {
		k++;
		coordinates->setAt(shellCoordinates->getAt(x),k);
	}
	for (unsigned int i = 0; i < holes.size(); i++) {
		CoordinateList* childCoordinates=((LinearRing *)holes[i])->getCoordinates();
		for (int j = 0; j < childCoordinates->getSize(); j++) {
			k++;
			coordinates->setAt(childCoordinates->getAt(j),k);
		}
	}
	return coordinates;
}

int Polygon::getNumPoints() {
	int numPoints = shell->getNumPoints();
	for (unsigned int i = 0; i < holes.size(); i++) {
		numPoints += ((LinearRing *)holes[i])->getNumPoints();
	}
	return numPoints;
}

int Polygon::getDimension() {
	return 2;
}

int Polygon::getBoundaryDimension() {
	return 1;
}

bool Polygon::isEmpty() {
	return shell->isEmpty();
}

bool Polygon::isSimple() {
	return true;
}

LineString* Polygon::getExteriorRing() {
	return shell;
}

int Polygon::getNumInteriorRing() {
	return (int)holes.size();
}

LineString* Polygon::getInteriorRingN(int n) {
	return (LineString *)holes[n];
}

string Polygon::getGeometryType() {
	return "Polygon";
}

Geometry* Polygon::getBoundary() {
	if (isEmpty()) {
		return new GeometryCollection(NULL, precisionModel, SRID);
	}
	vector<Geometry *> rings(holes.size() + 1);
	rings[0]=dynamic_cast<LineString *>(shell);
	for (unsigned int i=0; i<holes.size(); i++) {
		rings[i + 1] = ((LineString *)holes[i]);
	}
	return new MultiLineString(&rings, precisionModel, SRID);
}

Envelope* Polygon::computeEnvelopeInternal() {
	return &(shell->getEnvelopeInternal());
}

bool Polygon::equalsExact(Geometry *other) {
	if (!isEquivalentClass(other)) {
		return false;
	}
	Polygon otherPolygon(*dynamic_cast<Polygon*>(other));
	if (typeid(*shell)!=typeid(Geometry)) {
		return false;
	}
	Geometry thisShell(*dynamic_cast<Geometry *>(shell));
	if (typeid(*(otherPolygon.shell))!=typeid(Geometry)) {
		return false;
	}
	Geometry otherPolygonShell(*dynamic_cast<Geometry *>(otherPolygon.shell));
	if (!shell->equalsExact(&otherPolygonShell)) {
		return false;
	}
	if (holes.size()!=otherPolygon.holes.size()) {
		return false;
	}
	for (unsigned int i = 0; i < holes.size(); i++) {
		if (typeid(*holes[i])!=typeid(Geometry)) {
			return false;
		}
		if (typeid(*(otherPolygon.holes[i]))!=typeid(Geometry)) {
			return false;
		}
		if (!((LinearRing *)holes[i])->equalsExact(otherPolygon.holes[i])) {
			return false;
		}
	}
	return true;
}

void Polygon::apply(CoordinateFilter *filter) {
	shell->apply(filter);
	for (unsigned int i = 0; i < holes.size(); i++) {
		((LinearRing *)holes[i])->apply(filter);
	}
}

void Polygon::apply(GeometryFilter *filter) {
	filter->filter(this);
}

Geometry Polygon::convexHull() {
	return getExteriorRing()->convexHull();
}

void Polygon::normalize() {
	normalize(shell, true);
	for (unsigned int i = 0; i < holes.size(); i++) {
		normalize((LinearRing *)holes[i], false);
	}
	sort(holes.begin(),holes.end(),greaterThen);
}

int Polygon::compareToSameClass(Polygon *p) {
	return shell->compareToSameClass(p->shell);
}

void Polygon::normalize(LinearRing *ring, bool clockwise) {
	if (ring->isEmpty()) {
		return;
	}
	CoordinateList* uniqueCoordinates=ring->getCoordinates();
	uniqueCoordinates->deleteAt(uniqueCoordinates->getSize()-1);
	Coordinate* minCoordinate=ring->getCoordinates()->minCoordinate();
	Geometry::scroll(uniqueCoordinates, minCoordinate);
	uniqueCoordinates->add(uniqueCoordinates->getAt(0));
	ring->setPoints(uniqueCoordinates);
	if (cgAlgorithms->isCCW(ring->getCoordinates())==clockwise) {
		reversePointOrder(ring->getCoordinates());
	}
}

Coordinate* Polygon::getCoordinate() {
	return shell->getCoordinate();
}

/**
*  Returns the area of this <code>Polygon</code>
*
*@return the area of the polygon
*/
double Polygon::getArea() {
	double area=0.0;
	area+=fabs(CGAlgorithms::signedArea(shell->getCoordinates()));
	for(unsigned int i=0;i<holes.size();i++) {
//		area-=fabs(CGAlgorithms::signedArea(holes.at(i)->getCoordinates()));
        area-=fabs(CGAlgorithms::signedArea(holes[i]->getCoordinates()));
	}
	return area;
}

/**
*  Returns the perimeter of this <code>Polygon</code>
*
*@return the perimeter of the polygon
*/
double Polygon::getLength() {
	double len=0.0;
	len+=shell->getLength();
	for(unsigned int i=0;i<holes.size();i++) {
//		len+=holes.at(i)->getLength();
        len+=holes[i]->getLength();
	}
	return len;
}

void Polygon::apply(GeometryComponentFilter *filter) {
	shell->apply(filter);
	for(unsigned int i=0;i<holes.size();i++) {
//		holes.at(i)->apply(filter);
        holes[i]->apply(filter);
	}
}

Polygon::~Polygon(){}
