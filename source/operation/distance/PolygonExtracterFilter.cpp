#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<const Geometry*>* PolygonExtracterFilter::getPolygons(const Geometry *geom) {
	vector<const Geometry*> *cmp=new vector<const Geometry*>();
	PolygonExtracterFilter *p=new PolygonExtracterFilter(cmp);
	geom->apply_ro(p);
	delete p;
	return cmp;
}

PolygonExtracterFilter::PolygonExtracterFilter(vector<const Geometry*> *newComps){
	comps=newComps;
}

void PolygonExtracterFilter::filter_ro(const Geometry *geom) {
	if (typeid(*geom)==typeid(Polygon)) comps->push_back(geom);
}
}

