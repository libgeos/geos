#include "geom.h"
#include <typeinfo>

Polygon::Polygon(){}
Polygon::Polygon(const Polygon &p): Geometry(p.precisionModel, p.SRID), shell(p.shell), holes(p.holes) {
//	LinearRing s(p.shell);
//	vector<Geometry *> h(p.holes.begin(),p.holes.end());
//	shell=p.shell;
//	holes=p.holes;
}

Polygon::Polygon(LinearRing shell, PrecisionModel precisionModel, int SRID){
	Polygon(&shell, &vector<Geometry *>(), precisionModel, SRID);
}

Polygon::Polygon(LinearRing *newShell, vector<Geometry *> *newHoles,
				 PrecisionModel precisionModel, int SRID):
				Geometry(precisionModel, SRID) {
	if (newShell==NULL) {
		LinearRing newShell(CoordinateList(), precisionModel, SRID);
	}
	if (newHoles==NULL) {
		vector<Geometry *> *newHoles();
	}
	if (hasNullElements(*newHoles)) {
		throw "IllegalArgumentException: holes must not contain null elements";
	}
	if (newShell->isEmpty() && hasNonEmptyElements(*newHoles)) {
		throw "IllegalArgumentException: shell is empty but holes are not";
	}
	shell=*newShell;
	holes=*newHoles;
}

CoordinateList Polygon::getCoordinates() {
	if (isEmpty()) {
		return CoordinateList();
	}
	CoordinateList coordinates(getNumPoints());
	int k = -1;
	CoordinateList shellCoordinates(shell.getCoordinates());
	for (int x = 0; x < shellCoordinates.getSize(); x++) {
		k++;
		coordinates.setAt(shellCoordinates.getAt(x),k);
	}
	for (unsigned int i = 0; i < holes.size(); i++) {
		CoordinateList childCoordinates(((LinearRing *)holes[i])->getCoordinates());
		for (int j = 0; j < childCoordinates.getSize(); j++) {
			k++;
			coordinates.setAt(childCoordinates.getAt(j),k);
		}
	}
	return coordinates;
}

int Polygon::getNumPoints() {
	int numPoints = shell.getNumPoints();
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
	return shell.isEmpty();
}

bool Polygon::isSimple() {
	return true;
}

LineString Polygon::getExteriorRing() {
	return shell;
}

int Polygon::getNumInteriorRing() {
	return (int)holes.size();
}

LineString Polygon::getInteriorRingN(int n) {
	return *((LineString *)holes[n]);
}

string Polygon::getGeometryType() {
	return "Polygon";
}

Geometry Polygon::getBoundary() {
	if (isEmpty()) {
		return GeometryCollection(NULL, precisionModel, SRID);
	}
	vector<Geometry *> rings(holes.size() + 1);
	rings[0]=dynamic_cast<LineString *>(&shell);
	for (unsigned int i=0; i<holes.size(); i++) {
		rings[i + 1] = ((LineString *)holes[i]);
	}
	return MultiLineString(&rings, precisionModel, SRID);
}

Envelope Polygon::computeEnvelopeInternal() {
	return shell.getEnvelopeInternal();
}

bool Polygon::equalsExact(Geometry *other) {
	if (!isEquivalentClass(other)) {
		return false;
	}
	Polygon otherPolygon(*dynamic_cast<Polygon*>(other));
	if (typeid(shell)!=typeid(Geometry)) {
		return false;
	}
	Geometry thisShell(*dynamic_cast<Geometry *>(&shell));
	if (typeid(otherPolygon.shell)!=typeid(Geometry)) {
		return false;
	}
	Geometry otherPolygonShell(*dynamic_cast<Geometry *>(&(otherPolygon.shell)));
	if (!shell.equalsExact(&otherPolygonShell)) {
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
	shell.apply(filter);
	for (unsigned int i = 0; i < holes.size(); i++) {
		((LinearRing *)holes[i])->apply(filter);
	}
}

void Polygon::apply(GeometryFilter *filter) {
	filter->filter(this);
}

Geometry Polygon::convexHull() {
	return getExteriorRing().convexHull();
}

void Polygon::normalize() {
	normalize(&shell, true);
	for (unsigned int i = 0; i < holes.size(); i++) {
		normalize((LinearRing *)holes[i], false);
	}
	sort(holes.begin(),holes.end(),greaterThen);
}

bool Polygon::compareToSameClass(Polygon p) {
	return shell.compareToSameClass(p.shell);
}

//!!! External dependency
void Polygon::normalize(LinearRing *ring, bool clockwise) {
	if (ring->isEmpty()) {
		return;
	}
	CoordinateList uniqueCoordinates(ring->getCoordinates());
	uniqueCoordinates.deleteAt(uniqueCoordinates.getSize()-1);
	Coordinate minCoordinate=Geometry::minCoordinate(ring->getCoordinates());
	Geometry::scroll(&uniqueCoordinates, minCoordinate);
	uniqueCoordinates.add(uniqueCoordinates.getAt(0));
	ring->setPoints(uniqueCoordinates);
//!!! External dependency
//	if (cgAlgorithms.isCCW(ring.getCoordinates()) == clockwise) {
//		reversePointOrder(ring.getCoordinates());
//	}
}



Polygon::~Polygon(){}
