#include "../headers/geom.h"
#include <algorithm>
#include <typeinfo>
#include "../headers/geosAlgorithm.h"
#include "../headers/operation.h"

namespace geos {

LineString::LineString(){}

//Replaces clone()
LineString::LineString(const LineString &ls): Geometry(ls.precisionModel, ls.SRID) {
//	CoordinateList pts(ls.points);
//	points=pts;
//	for(int i=0;i<ls.points->getSize();i++) {
//		points->add(ls.points->getAt(i));
//	}
	points=CoordinateListFactory::internalFactory->createCoordinateList(ls.points);
}

LineString::LineString(CoordinateList *newPoints, PrecisionModel* precisionModel, int SRID):
						Geometry(precisionModel, SRID){
	if (newPoints==NULL) {
		newPoints=CoordinateListFactory::internalFactory->createCoordinateList();
	}
	if (hasNullElements(newPoints)) {
		delete newPoints;
		throw new IllegalArgumentException("point array must not contain null elements\n");
	}
	if (newPoints->getSize()==1) {
		delete newPoints;
		throw new IllegalArgumentException("point array must contain 0 or >1 elements\n");
	}
	points=newPoints;
}

LineString::~LineString(){
	delete points;
}

CoordinateList* LineString::getCoordinates() {
	return points;
}

Coordinate& LineString::getCoordinateN(int n) {
	return points->getAt(n);
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
	return points->getSize()==0;
}

int LineString::getNumPoints() {
	return points->getSize();
}

Point* LineString::getPointN(int n) {
	return new Point(points->getAt(n), getPrecisionModel(), SRID);
}

Point* LineString::getStartPoint() {
	if (isEmpty()) {
		return new Point();
	}
	return getPointN(0);
}

Point* LineString::getEndPoint() {
	if (isEmpty()) {
		return new Point();
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

bool LineString::isSimple(){
	auto_ptr<IsSimpleOp> iso(new IsSimpleOp());
	return iso->isSimple(this);
}

Geometry* LineString::getBoundary() {
	if (isEmpty()) {
		return new GeometryCollection(NULL, precisionModel, SRID);
	}
	if (isClosed()) {
		return new MultiPoint(NULL, precisionModel, SRID);
	}
	vector<Geometry*> *pts=new vector<Geometry*>();
	pts->push_back(getStartPoint());
	pts->push_back(getEndPoint());
	return new MultiPoint(pts,precisionModel, SRID);
}

bool LineString::isCoordinate(Coordinate& pt) {
	for (int i = 1; i < points->getSize(); i++) {
		if (points->getAt(i)==pt) {
			return true;
		}
	}
	return false;
}

Envelope* LineString::computeEnvelopeInternal() {
	if (isEmpty()) {
		return new Envelope();
	}
	double minx = points->getAt(0).x;
	double miny = points->getAt(0).y;
	double maxx = points->getAt(0).x;
	double maxy = points->getAt(0).y;
	for (int i = 1; i < points->getSize(); i++) {
		minx = min(minx, points->getAt(i).x); //min
		maxx = max(maxx, points->getAt(i).x);
		miny = min(miny, points->getAt(i).y);
		maxy = max(maxy, points->getAt(i).y);
	}
	return new Envelope(minx, maxx, miny, maxy);
}

bool LineString::equalsExact(Geometry *other, double tolerance) {
	if (!isEquivalentClass(other)) {
		return false;
	}
	LineString *otherLineString=dynamic_cast<LineString*>(other);
	if (points->getSize()!=otherLineString->points->getSize()) {
		return false;
	}
	for (int i = 0; i < points->getSize(); i++) {
		if (!equal(points->getAt(i),otherLineString->points->getAt(i),tolerance)) {
			return false;
		}
	}
	return true;
}

void LineString::apply(CoordinateFilter *filter) {
	for (int i = 0; i < points->getSize(); i++) {
		filter->filter(points->getAt(i));
	}
	}

void LineString::apply(GeometryFilter *filter) {
	filter->filter(this);
}

/**
* Normalizes a LineString.  A normalized linestring
* has the first point which is not equal to it's reflected point
* less than the reflected point.
*/
void LineString::normalize() {
	for (int i = 0; i < points->getSize()/2; i++) {
		int j = points->getSize() - 1 - i;
		if (!(points->getAt(i)==points->getAt(j))) {
			if (points->getAt(i).compareTo(points->getAt(j)) > 0) {
				CoordinateList::reverse(points);
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

int LineString::compareToSameClass(Geometry *ls) {
	LineString *line=(LineString*)ls;
	// MD - optimized implementation
	int i=0;
	int j=0;
	while(i<points->getSize() && j<line->points->getSize()) {
		int comparison=points->getAt(i).compareTo(line->points->getAt(j));
		if(comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<points->getSize()) {
		return 1;
	}
	if (j<line->points->getSize()) {
		return -1;
	}
	return 0;
//	return compare(*(points->toVector()),*(((LineString*)ls)->points->toVector()));
}

Coordinate* LineString::getCoordinate() {
	if (isEmpty()) return(new Coordinate());
	return &(points->getAt(0));
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

int LineString::compareTo(LineString *ls){
	if (isEmpty() && ls->isEmpty()) {
		return 0;
	}
	if (isEmpty()) {
		return -1;
	}
	if (ls->isEmpty()) {
		return 1;
	}
	return compareToSameClass(ls);
}

}