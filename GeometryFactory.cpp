#include "geom.h"

GeometryFactory::GeometryFactory(){}
GeometryFactory::GeometryFactory(PrecisionModel newPrecisionModel, int newSRID){}
GeometryFactory::~GeometryFactory(){}

MultiPoint GeometryFactory::createMultiPoint(CoordinateList coordinates){
	return MultiPoint();
}
