#include "../headers/geom.h"

LinearRing::LinearRing(): LineString() {}
LinearRing::LinearRing(const LinearRing &lr): LineString(lr.points, lr.precisionModel, lr.SRID) {}
LinearRing::LinearRing(CoordinateList* points,PrecisionModel* precisionModel,int SRID): 
    LineString(points, precisionModel, SRID) {
	if (!LineString::isEmpty() && !LineString::isClosed()) {
		throw new IllegalArgumentException("points must form a closed linestring");
    }
	if (!points->isEmpty() && (points->getSize()>=1 && points->getSize()<=3)) {
		throw new IllegalArgumentException("Number of points must be 0 or >3");
	}
}
LinearRing::~LinearRing(){}
bool LinearRing::isSimple() {
	return true;
}
string LinearRing::getGeometryType(){
	return "LinearRing";
}
bool LinearRing::isClosed() {
	return true;
}

void LinearRing::setPoints(CoordinateList* cl){
	points->setPoints(*(cl->toVector()));
}
