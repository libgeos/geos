#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<const Geometry*>* PointExtracterFilter::getPoints(const Geometry *geom) {
	vector<const Geometry*> *points=new vector<const Geometry*>();
	PointExtracterFilter *p=new PointExtracterFilter(points);
	geom->apply_ro(p);
	delete p;
	return points;
}

PointExtracterFilter::PointExtracterFilter(vector<const Geometry*> *newPts){
	pts=newPts;
}

void PointExtracterFilter::filter_ro(const Geometry *geom) {
	if (typeid(*geom)==typeid(Point)) pts->push_back(geom);
}
}

