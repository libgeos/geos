#include "geom.h"

MultiLineString::MultiLineString(){}
//	MultiLineString(LineString[] lineStrings, PrecisionModel precisionModel, int SRID);
MultiLineString::~MultiLineString(){}

int MultiLineString::getDimension() {
	return 1;
}

int MultiLineString::getBoundaryDimension() {
	if (isClosed()) {
		return Dimension::False;
	}
	return 0;
}

string MultiLineString::getGeometryType() {
	return "MultiLineString";
}

bool MultiLineString::isClosed() {
	if (isEmpty()) {
		return false;
	}
	for (unsigned int i = 0; i < geometries.size(); i++) {
		if (!(dynamic_cast<LineString *>(geometries[i])->isClosed())) {
			return false;
		}
	}
	return true;
}

//!!! External dependency
bool MultiLineString::isSimple(){
	return false;
	//	return (new IsSimpleOp()).isSimple(this);
}

//!!! External dependency
Geometry MultiLineString::getBoundary() {
	if (isEmpty()) {
		return GeometryCollection(NULL, precisionModel, SRID);
	}
//	GeometryGraph g = new GeometryGraph(0, this);
//	CoordinateList pts = g.getBoundaryPoints();

	CoordinateList pts; //To Compile
	GeometryFactory fact(precisionModel, SRID);
	return fact.createMultiPoint(pts);
}

bool MultiLineString::equalsExact(Geometry *other) {
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other);
}
