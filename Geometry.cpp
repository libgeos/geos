#include "geom.h"

Geometry::Geometry(){}

Geometry::Geometry(PrecisionModel newPrecisionModel, int newSRID) {
	precisionModel = newPrecisionModel;
	SRID = newSRID;
}

bool Geometry::isEquivalentClass(Geometry *other){
	return true;
}


Geometry::~Geometry(){}

bool greaterThen(Geometry *first, Geometry *second) {
	return first->compareTo(second);
}