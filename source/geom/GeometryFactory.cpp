#include "../headers/geom.h"
#include "../headers/util.h"
#include <typeinfo>

namespace geos {

GeometryFactory::GeometryFactory() {
	precisionModel=new PrecisionModel();
	SRID=0;
}

GeometryFactory::GeometryFactory(PrecisionModel* newPrecisionModel, int newSRID){
    precisionModel=new PrecisionModel(*newPrecisionModel);
    SRID=newSRID;
}
GeometryFactory::~GeometryFactory(){
	delete precisionModel;
}
  
Point* GeometryFactory::createPointFromInternalCoord(Coordinate* coord, Geometry *exemplar) {
	exemplar->getPrecisionModel()->makePrecise(coord);
	return new Point(*coord,exemplar->getPrecisionModel(),exemplar->getSRID());
}


Geometry* GeometryFactory::toGeometry(Envelope* envelope,PrecisionModel* precisionModel,int SRID) {
	if (envelope->isNull()) {
		return new Point(*(new Coordinate()),precisionModel,SRID);
	}
	if (envelope->getMinX()==envelope->getMaxX() && envelope->getMinY()==envelope->getMaxY()) {
		return new Point(*(new Coordinate(envelope->getMinX(),envelope->getMinY())),precisionModel,SRID);
	}
	CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList();
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMinY())));
	cl->add(*(new Coordinate(envelope->getMaxX(), envelope->getMinY())));
	cl->add(*(new Coordinate(envelope->getMaxX(), envelope->getMaxY())));
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMaxY())));
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMinY())));
	return new Polygon(new LinearRing(cl,precisionModel,SRID),precisionModel,SRID);
}

PrecisionModel* GeometryFactory::getPrecisionModel(){
	return precisionModel;
}

Point* GeometryFactory::createPoint(Coordinate& coordinate){
	return new Point(coordinate,precisionModel,SRID);
}

MultiLineString* GeometryFactory::createMultiLineString(vector<Geometry *> *lineStrings){
	return new MultiLineString(lineStrings,precisionModel,SRID);
}

GeometryCollection* GeometryFactory::createGeometryCollection(vector<Geometry *> *geometries){
	return new GeometryCollection(geometries,precisionModel,SRID);
}

MultiPolygon* GeometryFactory::createMultiPolygon(vector<Geometry *> *polygons){
	return new MultiPolygon(polygons,precisionModel,SRID);
}

LinearRing* GeometryFactory::createLinearRing(CoordinateList* coordinates) {
	if (coordinates->getSize()>0 && 
		!coordinates->getAt(0).equals2D(coordinates->getAt(coordinates->getSize() - 1))) {
			throw new IllegalArgumentException("LinearRing not closed");
	}
	return new LinearRing(coordinates, precisionModel, SRID);
}

MultiPoint* GeometryFactory::createMultiPoint(vector<Geometry *> *point) {
	return new MultiPoint(point,precisionModel,SRID);
}

MultiPoint* GeometryFactory::createMultiPoint(CoordinateList* coordinates) {
	vector<Geometry *> *pts=new vector<Geometry *>;
	for (int i=0; i<coordinates->getSize(); i++) {
		Point *pt=createPoint(coordinates->getAt(i));
		pts->push_back(pt);
	}
	return createMultiPoint(pts);
}

Polygon* GeometryFactory::createPolygon(LinearRing *shell, vector<Geometry *> *holes) {
	return new Polygon(shell, holes, precisionModel, SRID);
}

LineString* GeometryFactory::createLineString(CoordinateList* coordinates) {
	return new LineString(coordinates, precisionModel, SRID);
}

Geometry* GeometryFactory::buildGeometry(vector<Geometry *> *geoms) {
	string geomClass("NULL");
	bool isHeterogeneous=false;
	bool isCollection=geoms->size()>1;
	unsigned int i;
    
	for (i=0; i<geoms->size(); i++) {
		string partClass(typeid(*(*geoms)[i]).name());
		if (geomClass=="NULL") {
			geomClass=partClass;
		}
		if (partClass!=geomClass) {
			isHeterogeneous = true;
		}
	}
    // for the empty geometry, return an empty GeometryCollection
	if (geomClass=="NULL") {
		return createGeometryCollection(NULL);
	}
	if (isHeterogeneous) {
		return createGeometryCollection(geoms);
	}
	Geometry *geom0=(*geoms)[0];
	if (isCollection) {
		if (typeid(*geom0)==typeid(Polygon)) {
			return createMultiPolygon(geoms);
		} else if (typeid(*geom0)==typeid(LineString)) {
			return createMultiLineString(geoms);
		} else if (typeid(*geom0)==typeid(Point)) {
			return createMultiPoint(geoms);
		}
		Assert::shouldNeverReachHere();
	}
	if (typeid(*geom0)==typeid(Polygon))
		return new Polygon(*((Polygon*) geom0));
	// LineString also handles LinearRings
	else if (typeid(*geom0)==typeid(LineString))
		return new LineString(*((LineString*) geom0));
	else if (typeid(*geom0)==typeid(Point))
		return new Point(*((Point*) geom0));
	else if (typeid(*geom0)==typeid(MultiPoint))
		return new MultiPoint(*((MultiPoint*) geom0));
	else if (typeid(*geom0)==typeid(MultiLineString))
		return new MultiLineString(*((MultiLineString*) geom0));
	else if (typeid(*geom0)==typeid(MultiPolygon))
		return new MultiPolygon(*((MultiPolygon*) geom0));
	else if (typeid(*geom0)==typeid(GeometryCollection))
		return new GeometryCollection(*((GeometryCollection*) geom0));
	else 
		return geom0;
}

}