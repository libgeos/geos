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

LineString::LineString(const CoordinateList *pts, const PrecisionModel* pm,
		int SRID): Geometry(precisionModel, SRID){
	if (pts==NULL) {
		pts=CoordinateListFactory::internalFactory->createCoordinateList();
	}
	if (hasNullElements(pts)) {
		throw new IllegalArgumentException("point array must not contain null elements\n");
	}
	if (pts->getSize()==1) {
		throw new IllegalArgumentException("point array must contain 0 or >1 elements\n");
	}
	points=CoordinateListFactory::internalFactory->createCoordinateList(pts); // xie 
}

LineString::~LineString(){
	delete points;
}

CoordinateList* LineString::getCoordinates() const {
	return CoordinateListFactory::internalFactory->createCoordinateList(points); // callers must be free to delete returned value ! - strk
	//return points;
}

const Coordinate& LineString::getCoordinateN(int n) const {
	return points->getAt(n);
}

int LineString::getDimension() const {
	return 1;
}

int LineString::getBoundaryDimension() const {
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

bool LineString::isEmpty() const {
	return points->getSize()==0;
}

int LineString::getNumPoints() const {
	return points->getSize();
}

Point* LineString::getPointN(int n) const {
	return new Point(points->getAt(n), getPrecisionModel(), SRID);
}

Point* LineString::getStartPoint() const {
	if (isEmpty()) {
		return new Point();
	}
	return getPointN(0);
}

Point* LineString::getEndPoint() const {
	if (isEmpty()) {
		return new Point();
	}
	return getPointN(getNumPoints() - 1);
}

bool LineString::isClosed() const {
	if (isEmpty()) {
		return false;
	}
	return getCoordinateN(0).equals2D(getCoordinateN(getNumPoints()-1));
}

bool LineString::isRing() const {
	return isClosed() && isSimple();
}

string LineString::getGeometryType() const {
	return "LineString";
}

bool LineString::isSimple() const {
	auto_ptr<IsSimpleOp> iso(new IsSimpleOp());
	return iso->isSimple(this);
}

Geometry* LineString::getBoundary() const {
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

bool LineString::isCoordinate(Coordinate& pt) const {
	for (int i = 1; i < points->getSize(); i++) {
		if (points->getAt(i)==pt) {
			return true;
		}
	}
	return false;
}

Envelope* LineString::computeEnvelopeInternal() const {
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

bool LineString::equalsExact(const Geometry *other, double tolerance) const {
	if (!isEquivalentClass(other)) {
		return false;
	}
	const LineString *otherLineString=dynamic_cast<const LineString*>(other);
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

void LineString::apply_rw(CoordinateFilter *filter) {
	for (int i = 0; i < points->getSize(); i++) {
		Coordinate newcoord = points->getAt(i);
		filter->filter_rw(newcoord);
		points->setAt(newcoord, i);
	}
}

void LineString::apply_ro(CoordinateFilter *filter) const {
	for (int i = 0; i < points->getSize(); i++) {
		// getAt returns a 'const' coordinate
		filter->filter_ro(points->getAt(i));
	}
}

void LineString::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void LineString::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
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

bool LineString::isEquivalentClass(const Geometry *other) const {
	if (typeid(*other)==typeid(LineString))
		return true;
	else 
		return false;
}

int LineString::compareToSameClass(const Geometry *ls) const {
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

const Coordinate* LineString::getCoordinate() const
{
	// should use auto_ptr here or return NULL or throw an exception !
	// 	--strk;
	if (isEmpty()) return(new Coordinate());
	return &(points->getAt(0));
}

/**
*  Returns the length of this <code>LineString</code>
*
*@return the area of the polygon
*/
double LineString::getLength() const {
	return CGAlgorithms::length(points);
}

void LineString::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}

void LineString::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

int LineString::compareTo(const LineString *ls) const {
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

