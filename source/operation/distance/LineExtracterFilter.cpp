#include "../../headers/opDistance.h"
#include <typeinfo>

vector<Geometry*>* LineExtracterFilter::getLines(Geometry *geom) {
	vector<Geometry*> *lns=new vector<Geometry*>();
	geom->apply(new LineExtracterFilter(lns));
	return lns;
}

LineExtracterFilter::LineExtracterFilter(vector<Geometry*> *newLines){
	lines=newLines;
}

void LineExtracterFilter::filter(Geometry *geom) {
	if (typeid(*geom)==typeid(LineString)) lines->push_back(geom);
}
