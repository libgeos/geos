#include "../headers/geom.h"
#include "../headers/util.h"

namespace geos {

LinearRing::LinearRing(): LineString() {}
//LinearRing::LinearRing(const LinearRing &lr): LineString(lr.points, lr.precisionModel, lr.SRID) {}
LinearRing::LinearRing(const LinearRing &lr): LineString(lr) {}
LinearRing::LinearRing(CoordinateList* points,PrecisionModel* precisionModel,int SRID): 
    LineString(points, precisionModel, SRID) {
	if (!LineString::isEmpty() && !LineString::isClosed()) {
		delete points;
		throw new IllegalArgumentException("points must form a closed linestring");
    }
	if (!points->isEmpty() && (points->getSize()>=1 && points->getSize()<=3)) {
		delete points;
		throw new IllegalArgumentException("Number of points must be 0 or >3");
	}
}
LinearRing::~LinearRing(){
//	delete points;
}
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
	vector<Coordinate> *v=cl->toVector();
	points->setPoints(*(v));
	delete v;
}
}

