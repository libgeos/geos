#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<Coordinate*>* ConnectedElementPointFilter::getCoordinates(Geometry *geom) {
	vector<Coordinate*> *points=new vector<Coordinate*>();
	ConnectedElementPointFilter *c=new ConnectedElementPointFilter(points);
	geom->apply(c);
	delete c;
	return points;
}

ConnectedElementPointFilter::ConnectedElementPointFilter(vector<Coordinate*> *newPts){
	pts=newPts;
}

void ConnectedElementPointFilter::filter(Geometry *geom) {
	if ((typeid(*geom)==typeid(Point)) ||
		(typeid(*geom)==typeid(LineString)) ||
		(typeid(*geom)==typeid(Polygon)))
			pts->push_back(geom->getCoordinate());
}
}

