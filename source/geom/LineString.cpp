#include "geom.h"
#include <algorithm>
#include <typeinfo>
#include "geosAlgorithm.h"


LineString::LineString(){
	CoordinateList points;
}

//Replaces clone()
LineString::LineString(const LineString &ls): Geometry(ls.precisionModel, ls.SRID), points(ls.points) {
//	CoordinateList pts(ls.points);
//	points=pts;
//	points=ls.points;
}

LineString::LineString(CoordinateList newPoints, PrecisionModel precisionModel, int SRID):
						Geometry(precisionModel, SRID), points(newPoints) {
	// Can't be null
	//if (points == null) {
	//	points = new Coordinate[]{};
	//}
	if (hasNullElements(newPoints)) {
		throw "IllegalArgumentException: point array must not contain null elements\n";
	}
	if (newPoints.getSize()==1) {
		throw "IllegalArgumentException: point array must contain 0 or >1 elements\n";
	}
//	CoordinateList pts(newPoints);
//	points=pts;
//	points=CoordinateList(newPoints);
//?	points=newPoints;
}

LineString::~LineString(){}

CoordinateList LineString::getCoordinates() {
	return points;
}

Coordinate LineString::getCoordinateN(int n) {
	return points.getAt(n);
}

int LineString::getDimension() {
	return 1;
}

int LineString::getBoundaryDimension() {
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

bool LineString::isEmpty() {
	return points.getSize()==0;
}

int LineString::getNumPoints() {
	return points.getSize();
}

Point LineString::getPointN(int n) {
	return Point(points.getAt(n), getPrecisionModel(), SRID);
}

Point LineString::getStartPoint() {
	if (isEmpty()) {
		return Point();
	}
	return getPointN(0);
}

Point LineString::getEndPoint() {
	if (isEmpty()) {
		return Point();
	}
	return getPointN(getNumPoints() - 1);
}

bool LineString::isClosed() {
	if (isEmpty()) {
		return false;
	}
	return getCoordinateN(0).equals2D(getCoordinateN(getNumPoints()-1));
}

bool LineString::isRing() {
	return isClosed() && isSimple();
}

string LineString::getGeometryType() {
	return "LineString";
}

//!!! External dependency
bool LineString::isSimple(){
	return false;
	//	return (new IsSimpleOp()).isSimple(this);
}

Geometry LineString::getBoundary() {
	if (isEmpty()) {
		return GeometryCollection(NULL, precisionModel, SRID);
	}
	if (isClosed()) {
		return MultiPoint(NULL, precisionModel, SRID);
	}
//!!! to compile
	return MultiPoint(NULL, precisionModel, SRID);

//	return MultiPoint(new Point[]={getStartPoint(), getEndPoint()}, precisionModel, SRID);
}

bool LineString::isCoordinate(Coordinate pt) {
	for (int i = 1; i < points.getSize(); i++) {
		if (points.getAt(i)==pt) {
			return true;
		}
	}
	return false;
}

Envelope LineString::computeEnvelopeInternal() {
	if (isEmpty()) {
		return Envelope();
	}
	double minx = points.getAt(0).x;
	double miny = points.getAt(0).y;
	double maxx = points.getAt(0).x;
	double maxy = points.getAt(0).y;
	for (int i = 1; i < points.getSize(); i++) {
		minx = min(minx, points.getAt(i).x); //min
		maxx = max(maxx, points.getAt(i).x);
		miny = min(miny, points.getAt(i).y);
		maxy = max(maxy, points.getAt(i).y);
	}
	return Envelope(minx, maxx, miny, maxy);
}

bool LineString::equalsExact(Geometry *other) {
	if (!isEquivalentClass(other)) {
		return false;
	}
	LineString *otherLineString=dynamic_cast<LineString*>(other);
	if (points.getSize()!=otherLineString->points.getSize()) {
		return false;
	}
	for (int i = 0; i < points.getSize(); i++) {
		if (!(points.getAt(i)==otherLineString->points.getAt(i))) {
			return false;
		}
	}
	return true;
}

void LineString::apply(CoordinateFilter *filter) {
	for (int i = 0; i < points.getSize(); i++) {
		filter->filter(points.getAt(i));
	}
	}

void LineString::apply(GeometryFilter *filter) {
	filter->filter(this);
}

void LineString::normalize() {
	for (int i = 0; i < points.getSize(); i++) {
		int j = points.getSize() - 1 - i;
		if (!(points.getAt(i)==points.getAt(j))) {
			if (points.getAt(i).compareTo(points.getAt(j)) > 0) {
				reversePointOrder(&points);
			}
			return;
		}
	}
}

bool LineString::isEquivalentClass(Geometry *other) {
	if (typeid(*other)==typeid(LineString))
		return true;
	else 
		return false;
}

int LineString::compareToSameClass(LineString *ls) {
	return compare(points.toVector(),ls->points.toVector());
}

Coordinate LineString::getCoordinate() {
	if (isEmpty()) return Coordinate();
	return points.getAt(0);
}

/**
*  Returns the length of this <code>LineString</code>
*
*@return the area of the polygon
*/
double LineString::getLength() {
	return CGAlgorithms::length(points);
}

void LineString::apply(GeometryComponentFilter *filter) {
	filter->filter(this);
}

