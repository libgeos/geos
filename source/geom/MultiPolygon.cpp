#include "geom.h"

MultiPolygon::MultiPolygon(){}
MultiPolygon::MultiPolygon(vector<Geometry *> *polygons, PrecisionModel* precisionModel, int SRID):
GeometryCollection(polygons, precisionModel, SRID){}
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

Geometry* MultiPolygon::getBoundary() {
	if (isEmpty()) {
		return new GeometryCollection(NULL, precisionModel, SRID);
	}
	vector<Geometry *>* allRings=new vector<Geometry *>();
	for (unsigned int i = 0; i < geometries->size(); i++) {
		Polygon *pg=(Polygon *) (*geometries)[i];
		GeometryCollection* rings=dynamic_cast<GeometryCollection *>(pg->getBoundary());
		for (int j = 0; j < rings->getNumGeometries(); j++) {
			allRings->push_back(rings->getGeometryN(j));
		}
	}
//LineString[] allRingsArray = new LineString[allRings.size()];
	return new MultiLineString(allRings,precisionModel,SRID);
}

bool MultiPolygon::equalsExact(Geometry *other) {
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other);
}
