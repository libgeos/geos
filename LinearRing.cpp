#include "geom.h"

LinearRing::LinearRing(): LineString() {}
LinearRing::LinearRing(const LinearRing &lr): LineString(lr.points, lr.precisionModel, lr.SRID) {}
LinearRing::LinearRing(CoordinateList points,PrecisionModel precisionModel,int SRID): 
    LineString(points, precisionModel, SRID) {
	if (!LineString::isEmpty() && ! LineString::isClosed()) {
		throw "IllegalArgumentException: points must form a closed linestring";
    }
	if (!points.isEmpty() && (points.getSize() == 1 || points.getSize() == 2)) {
		throw "IllegalArgumentException:points must contain 0 or >2 elements";
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
