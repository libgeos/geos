#include "../headers/geom.h"
#include "../headers/util.h"

namespace geos {

Point::Point(){
	coordinate.setNull();
}
Point::Point(Coordinate& c, PrecisionModel* precisionModel, int SRID): Geometry(precisionModel,SRID) {
	coordinate=c;
}

Point::Point(const Point &p): Geometry(p.precisionModel,p.SRID) {
	coordinate=p.coordinate;
}

CoordinateList* Point::getCoordinates() {
	if (isEmpty()) {
		return CoordinateListFactory::internalFactory->createCoordinateList();
	} else {
		return CoordinateListFactory::internalFactory->createCoordinateList(coordinate);
	}
}
int Point::getNumPoints() {
	return isEmpty() ? 0 : 1;
}

bool Point::isEmpty() {
	return coordinate == Coordinate::getNull();
}

bool Point::isSimple() {return true;}
bool Point::isValid() {return true;}
int Point::getDimension() {return 0;}
int Point::getBoundaryDimension() {return Dimension::False;}

double Point::getX() {
	if (isEmpty()) {
		throw new UnsupportedOperationException("getX called on empty Point\n");
	}
	return coordinate.x;
}

double Point::getY() {
	if (isEmpty()) {
		throw new UnsupportedOperationException("getY called on empty Point\n");
	}
	return coordinate.y;
}

Coordinate* Point::getCoordinate() {return &coordinate;}

string Point::getGeometryType() {
	return "Point";
}

Geometry* Point::getBoundary() {
	return new GeometryCollection(NULL, precisionModel, SRID);
}

Envelope* Point::computeEnvelopeInternal() {
	if (isEmpty()) {
		return new Envelope();
	}
	return new Envelope(coordinate.x, coordinate.x, coordinate.y, coordinate.y);
}

void Point::apply(CoordinateFilter *filter) {
    if (isEmpty()) {return;}
	filter->filter(coordinate);
}

void Point::apply(GeometryFilter *filter) {
	filter->filter(this);
}

void Point::apply(GeometryComponentFilter *filter) {
	filter->filter(this);
}

bool Point::equalsExact(Geometry *other, double tolerance) {
	if (!isEquivalentClass(other)) {
		return false;
	}
	if (isEmpty() && other->isEmpty()) {
		return true;
	}
    return equal(((Point*) other)->coordinate, coordinate, tolerance);
}

int Point::compareToSameClass(Geometry *point) {
	return coordinate.compareTo(*(((Point*)point)->getCoordinate()));
}

Point::~Point(){
}
}