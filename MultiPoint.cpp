#include "geom.h"

MultiPoint::MultiPoint(){}
MultiPoint::MultiPoint(vector<Geometry *> *points,PrecisionModel pm, int SRID):
GeometryCollection(points, pm, SRID){}
MultiPoint::~MultiPoint(){}

int MultiPoint::getDimension() {
	return 0;
}

int MultiPoint::getBoundaryDimension() {
	return Dimension::False;
}

string MultiPoint::getGeometryType() {
	return "MultiPoint";
}

Geometry MultiPoint::getBoundary() {
	return GeometryCollection(NULL, precisionModel, SRID);
}

//!!! External dependency
bool MultiPoint::isSimple(){
	return false;
	//	return (new IsSimpleOp()).isSimple(this);
}

bool MultiPoint::isValid() {
	return true;
}

bool MultiPoint::equalsExact(Geometry *other) {
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other);
  }

Coordinate MultiPoint::getCoordinate(int n) {
	return ((Point *)geometries[n])->getCoordinate();
}