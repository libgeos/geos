#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<const Geometry*>* LineExtracterFilter::getLines(const Geometry *geom) {
	vector<const Geometry*> *lns=new vector<const Geometry*>();
	LineExtracterFilter *l=new LineExtracterFilter(lns);
	geom->apply_ro(l);
	delete l;
	return lns;
}

LineExtracterFilter::LineExtracterFilter(vector<const Geometry*> *newLines){
	lines=newLines;
}

void LineExtracterFilter::filter_ro(const Geometry *geom) {
	if (typeid(*geom)==typeid(LineString) || typeid(*geom)==typeid(LinearRing)) lines->push_back(geom);
}
}

