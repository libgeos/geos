#include "../../headers/opDistance.h"
#include <typeinfo>

vector<Coordinate*>* ConnectedElementPointFilter::getCoordinates(Geometry *geom) {
	vector<Coordinate*> *points=new vector<Coordinate*>();
	geom->apply(new ConnectedElementPointFilter(points));
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
