#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<Geometry*>* PolygonExtracterFilter::getPolygons(Geometry *geom) {
	vector<Geometry*> *cmp=new vector<Geometry*>();
	PolygonExtracterFilter *p=new PolygonExtracterFilter(cmp);
	geom->apply(p);
	delete p;
	return cmp;
}

PolygonExtracterFilter::PolygonExtracterFilter(vector<Geometry*> *newComps){
	comps=newComps;
}

void PolygonExtracterFilter::filter(Geometry *geom) {
	if (typeid(*geom)==typeid(Polygon)) comps->push_back(geom);
}
}

