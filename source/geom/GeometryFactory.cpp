#include "geom.h"
#include "util.h"
#include <typeinfo>

GeometryFactory::GeometryFactory(): precisionModel(PrecisionModel()){
	SRID=0;
}

GeometryFactory::GeometryFactory(PrecisionModel newPrecisionModel, int newSRID){
    precisionModel=newPrecisionModel;
    SRID=newSRID;
}
GeometryFactory::~GeometryFactory(){}

Geometry GeometryFactory::toGeometry(Envelope envelope, PrecisionModel precisionModel,int SRID) {
	if (envelope.isNull()) {
		return Point(Coordinate(), precisionModel, SRID);
	}
	if (envelope.getMinX()==envelope.getMaxX() && envelope.getMinY()==envelope.getMaxY()) {
		return Point(Coordinate(envelope.getMinX(),envelope.getMinY()),precisionModel,SRID);
	}
	CoordinateList *cl=new BasicCoordinateList();
	cl->add(Coordinate(envelope.getMinX(), envelope.getMinY()));
	cl->add(Coordinate(envelope.getMaxX(), envelope.getMinY()));
	cl->add(Coordinate(envelope.getMaxX(), envelope.getMaxY()));
	cl->add(Coordinate(envelope.getMinX(), envelope.getMaxY()));
	cl->add(Coordinate(envelope.getMinX(), envelope.getMinY()));
	return Polygon(new LinearRing(cl,precisionModel,SRID),precisionModel,SRID);
}

PrecisionModel GeometryFactory::getPrecisionModel(){
	return precisionModel;
}

Point GeometryFactory::createPoint(Coordinate coordinate){
	return Point(coordinate,precisionModel,SRID);
}

MultiLineString GeometryFactory::createMultiLineString(vector<Geometry *> *lineStrings){
	return MultiLineString(lineStrings,precisionModel,SRID);
}

GeometryCollection GeometryFactory::createGeometryCollection(vector<Geometry *> *geometries){
	return GeometryCollection(geometries,precisionModel,SRID);
}

MultiPolygon GeometryFactory::createMultiPolygon(vector<Geometry *> *polygons){
	return MultiPolygon(polygons,precisionModel,SRID);
}

LinearRing GeometryFactory::createLinearRing(CoordinateList* coordinates) {
	if (coordinates->getSize()>0 && 
		!coordinates->getAt(0).equals2D(coordinates->getAt(coordinates->getSize() - 1))) {
			throw "IllegalArgumentException: LinearRing not closed";
	}
	return LinearRing(coordinates, precisionModel, SRID);
}

MultiPoint GeometryFactory::createMultiPoint(vector<Geometry *> *point) {
	return MultiPoint(point,precisionModel,SRID);
}

MultiPoint GeometryFactory::createMultiPoint(CoordinateList* coordinates) {
	vector<Geometry *> *pts=new vector<Geometry *>;
	for (int i=0; i<coordinates->getSize(); i++) {
		Point *pt=new Point(createPoint(coordinates->getAt(i)));
		pts->push_back(pt);
	}
	return createMultiPoint(pts);
}

Polygon GeometryFactory::createPolygon(LinearRing *shell, vector<Geometry *> *holes) {
	return Polygon(shell, holes, precisionModel, SRID);
}

LineString GeometryFactory::createLineString(CoordinateList* coordinates) {
	return LineString(coordinates, precisionModel, SRID);
}

Geometry GeometryFactory::buildGeometry(vector<Geometry *> geoms) {
	string geomClass("NULL");
	bool isHeterogeneous=false;
	bool isCollection=geoms.size()>1;
    
	for (unsigned int i=0; i<geoms.size(); i++) {
		string partClass(typeid(*geoms[i]).name());
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
		return createGeometryCollection(&geoms);
	}
	Geometry *geom0=geoms[0];
	if (isCollection) {
		if (typeid(*geom0)==typeid(Polygon)) {
			return createMultiPolygon(&geoms);
		} else if (typeid(*geom0)==typeid(LineString)) {
			return createMultiLineString(&geoms);
		} else if (typeid(*geom0)==typeid(Point)) {
			return createMultiPoint(&geoms);
		}
		Assert::shouldNeverReachHere();
	}
	return Geometry(*(geoms[0]));
}

