/*
* $Log$
* Revision 1.12  2003/10/11 03:23:22  strk
* fixed spurious typos
*
*/
#include "../headers/geom.h"
#include "../headers/util.h"

namespace geos {

LinearRing::LinearRing(): LineString() {}
//LinearRing::LinearRing(const LinearRing &lr): LineString(lr.points, lr.precisionModel, lr.SRID) {}
LinearRing::LinearRing(const LinearRing &lr): LineString(lr) {}

LinearRing::LinearRing(const CoordinateList* points, const PrecisionModel* pm,
		int SRID): LineString(points, pm, SRID) {
	if (!LineString::isEmpty() && !LineString::isClosed()) {
		throw new IllegalArgumentException("points must form a closed linestring");
    }
	if (!points->isEmpty() && (points->getSize()>=1 && points->getSize()<=3)) {
		throw new IllegalArgumentException("Number of points must be 0 or >3");
	}
}

// superclass LineString will delete internal CoordinateList
LinearRing::~LinearRing(){
}

bool LinearRing::isSimple() const {
	return true;
}
string LinearRing::getGeometryType() const {
	return "LinearRing";
}
bool LinearRing::isClosed() const {
	return true;
}

void LinearRing::setPoints(CoordinateList* cl){
	vector<Coordinate> *v=cl->toVector();
	points->setPoints(*(v));
	delete v;
}
}

