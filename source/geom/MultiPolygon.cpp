#include "../headers/geom.h"

namespace geos {

MultiPolygon::MultiPolygon(){}
MultiPolygon::MultiPolygon(vector<Geometry *> *polygons, PrecisionModel* precisionModel, int SRID):
GeometryCollection(polygons, precisionModel, SRID){}
MultiPolygon::~MultiPolygon(){}

int MultiPolygon::getDimension() const {
	return 2;
}

int MultiPolygon::getBoundaryDimension() const {
	return 1;
}

string MultiPolygon::getGeometryType() const {
	return "MultiPolygon";
}

bool MultiPolygon::isSimple() const {
	return true;
}

Geometry* MultiPolygon::getBoundary() const {
	if (isEmpty()) {
		return new GeometryCollection(NULL, precisionModel, SRID);
	}
	vector<Geometry *>* allRings=new vector<Geometry *>();
	for (unsigned int i = 0; i < geometries->size(); i++) {
		Polygon *pg=(Polygon *) (*geometries)[i];
		Geometry *g=pg->getBoundary();
		GeometryCollection* rings=(GeometryCollection*)g;
		for (int j = 0; j < rings->getNumGeometries(); j++) {
			allRings->push_back(new LineString(*(LineString*)rings->getGeometryN(j)));
		}
		delete g;
	}
//LineString[] allRingsArray = new LineString[allRings.size()];
	Geometry *ret=new MultiLineString(allRings,precisionModel,SRID);
//	delete allRings;
	return ret;
}

bool
MultiPolygon::equalsExact(const Geometry *other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
      return false;
    }
	return GeometryCollection::equalsExact(other, tolerance);
}
}

