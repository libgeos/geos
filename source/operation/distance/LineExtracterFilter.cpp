#include "../../headers/opDistance.h"
#include <typeinfo>

namespace geos {

vector<Geometry*>* LineExtracterFilter::getLines(Geometry *geom) {
	vector<Geometry*> *lns=new vector<Geometry*>();
	LineExtracterFilter *l=new LineExtracterFilter(lns);
	geom->apply(l);
	delete l;
	return lns;
}

LineExtracterFilter::LineExtracterFilter(vector<Geometry*> *newLines){
	lines=newLines;
}

void LineExtracterFilter::filter(Geometry *geom) {
	if (typeid(*geom)==typeid(LineString) || typeid(*geom)==typeid(LinearRing)) lines->push_back(geom);
}
}

