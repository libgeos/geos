#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<Geometry*>* PointExtracterFilter::getPoints(Geometry *geom) {
	vector<Geometry*> *points=new vector<Geometry*>();
	PointExtracterFilter *p=new PointExtracterFilter(points);
	geom->apply(p);
	delete p;
	return points;
}

PointExtracterFilter::PointExtracterFilter(vector<Geometry*> *newPts){
	pts=newPts;
}

void PointExtracterFilter::filter(Geometry *geom) {
	if (typeid(*geom)==typeid(Point)) pts->push_back(geom);
}
}

