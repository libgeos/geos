#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<const Coordinate*>* ConnectedElementPointFilter::getCoordinates(const Geometry *geom) {
	vector<const Coordinate*> *points=new vector<const Coordinate*>();
	ConnectedElementPointFilter *c=new ConnectedElementPointFilter(points);
	geom->apply_ro(c);
	delete c;
	return points;
}

ConnectedElementPointFilter::ConnectedElementPointFilter(vector<const Coordinate*> *newPts){
	pts=newPts;
}

void ConnectedElementPointFilter::filter_ro(const Geometry *geom) {
	if ((typeid(*geom)==typeid(Point)) ||
		(typeid(*geom)==typeid(LineString)) ||
		(typeid(*geom)==typeid(Polygon)))
			pts->push_back(geom->getCoordinate());
}
}

