#include "geom.h"

MultiPolygon::MultiPolygon(){}
//	MultiPolygon(Polygon[] polygons, PrecisionModel precisionModel, int SRID);
MultiPolygon::~MultiPolygon(){}

int MultiPolygon::getDimension() {
	return 2;
}

int MultiPolygon::getBoundaryDimension() {
	return 1;
}

string MultiPolygon::getGeometryType() {
	return "MultiPolygon";
}

bool MultiPolygon::isSimple(){
	return true;
}

Geometry MultiPolygon::getBoundary() {
	if (isEmpty()) {
		return GeometryCollection(NULL, precisionModel, SRID);
	}
//ArrayList allRings = new ArrayList();
	for (unsigned int i = 0; i < geometries.size(); i++) {
		Polygon pg(*dynamic_cast<Polygon *>(geometries[i]));
		GeometryCollection rings(*dynamic_cast<GeometryCollection *>(&pg.getBoundary()));
		for (int j = 0; j < rings.getNumGeometries(); j++) {
//allRings.add(rings.getGeometryN(j));
		}
	}
//LineString[] allRingsArray = new LineString[allRings.size()];
//	return MultiLineString((LineString[]) allRings.toArray(allRingsArray),precisionModel, SRID);
	return MultiLineString();
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

bool MultiPolygon::equalsExact(Geometry *other) {
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other);
}
