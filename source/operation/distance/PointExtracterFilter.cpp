#include "../../headers/opDistance.h"
#include <typeinfo>

vector<Geometry*>* PointExtracterFilter::getPoints(Geometry *geom) {
	vector<Geometry*> *points=new vector<Geometry*>();
	geom->apply(new PointExtracterFilter(points));
	return points;
}

PointExtracterFilter::PointExtracterFilter(vector<Geometry*> *newPts){
	pts=newPts;
}

void PointExtracterFilter::filter(Geometry *geom) {
	if (typeid(*geom)==typeid(Point)) pts->push_back(geom);
}
