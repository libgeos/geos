#include "geom.h"

GeometryCollection::GeometryCollection(){}
GeometryCollection::GeometryCollection(const GeometryCollection &gc){}
GeometryCollection::GeometryCollection(Geometry *geometry,PrecisionModel pm,int b){}
GeometryCollection::~GeometryCollection(void){}
bool GeometryCollection::equalsExact(Geometry *other){return false;}
int GeometryCollection::getNumGeometries(){
	return 0;
}
