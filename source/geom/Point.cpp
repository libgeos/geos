#include "../headers/geom.h"
#include "../headers/util.h"

namespace geos {

Point::Point(){
	coordinate.setNull();
}
Point::Point(const Coordinate& c, const PrecisionModel* precisionModel,
		int SRID): Geometry(precisionModel,SRID) {
	coordinate=c;
}

Point::Point(const Point &p): Geometry(p.precisionModel,p.SRID) {
	coordinate=p.coordinate;
}

CoordinateList* Point::getCoordinates() const {
	if (isEmpty()) {
		return CoordinateListFactory::internalFactory->createCoordinateList();
	} else {
		return CoordinateListFactory::internalFactory->createCoordinateList(coordinate);
	}
}
int Point::getNumPoints() const {
	return isEmpty() ? 0 : 1;
}

bool Point::isEmpty() const {
	return coordinate == Coordinate::getNull();
}

bool Point::isSimple() const {return true;}
bool Point::isValid() const {return true;}
int Point::getDimension() const {return 0;}
int Point::getBoundaryDimension() const {return Dimension::False;}

double Point::getX() const {
	if (isEmpty()) {
		throw new UnsupportedOperationException("getX called on empty Point\n");
	}
	return coordinate.x;
}

double Point::getY() const {
	if (isEmpty()) {
		throw new UnsupportedOperationException("getY called on empty Point\n");
	}
	return coordinate.y;
}

const Coordinate* Point::getCoordinate() const {return &coordinate;}

string Point::getGeometryType() const {
	return "Point";
}

Geometry* Point::getBoundary() const {
	return new GeometryCollection(NULL, precisionModel, SRID);
}

Envelope* Point::computeEnvelopeInternal() const {
	if (isEmpty()) {
		return new Envelope();
	}
	return new Envelope(coordinate.x, coordinate.x, coordinate.y, coordinate.y);
}

void Point::apply_ro(CoordinateFilter *filter) const {
    if (isEmpty()) {return;}
	filter->filter_ro(coordinate);
}

void Point::apply_rw(CoordinateFilter *filter) {
}

void Point::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Point::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

void Point::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}

void Point::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

bool Point::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}
	if (isEmpty() && other->isEmpty()) {
		return true;
	}
    return equal(((Point*) other)->coordinate, coordinate, tolerance);
}

int Point::compareToSameClass(const Geometry *point) const {
	return coordinate.compareTo(*(((Point*)point)->getCoordinate()));
}

Point::~Point(){
}
}

