#include "../../headers/opRelate.h"
#include "stdio.h"
#include <typeinfo>

namespace geos {

IntersectionMatrix* RelateOp::relate(Geometry *a,Geometry *b) {
	if (isBaseGeometryCollection(a) || isBaseGeometryCollection(b)) {
		return relateGC(toList(a),toList(b));
	}else {
		RelateOp *relOp=new RelateOp(a,b);
		IntersectionMatrix *im=relOp->getIntersectionMatrix();
		delete relOp;
		return im;
	}
}

/**
* Implements relate on GeometryCollections as the sum of
* the Intersection matrices for the components of the
* collection(s).
* This may or may not be appropriate semantics for this operation.
* @param a a List of Geometries, none of which are a basic GeometryCollection
* @param b a List of Geometries, none of which are a basic GeometryCollection
* @return the matrix representing the topological relationship of the geometries
*/
IntersectionMatrix* RelateOp::relateGC(vector<const Geometry*> *a,vector<const Geometry*> *b) {
	IntersectionMatrix *finalIM=new IntersectionMatrix();
	for(int i=0;i<(int)a->size();i++) {
		const Geometry *aGeom=(*a)[i];
		for(int j=0;j<(int)b->size();j++) {
			const Geometry *bGeom=(*b)[j];
			RelateOp *relOp=new RelateOp(aGeom,bGeom);
			IntersectionMatrix *im=relOp->getIntersectionMatrix();
			finalIM->add(im);
		}
	}
	return finalIM;
}

RelateOp::RelateOp(const Geometry *g0, const Geometry *g1):GeometryGraphOperation(g0,g1) {
	relateComp=new RelateComputer(arg);
}

RelateOp::~RelateOp() {
	delete relateComp;
}

IntersectionMatrix* RelateOp::getIntersectionMatrix() {
	return relateComp->computeIM();
}

vector<const Geometry*>* RelateOp::toList(const Geometry *geom) {
	vector<const Geometry*> *geomList=new vector<const Geometry*>();
	return addToList(geom,geomList);
}

vector<const Geometry*>* RelateOp::addToList(const Geometry *geom, vector<const Geometry*>* geomList) {
	if (isBaseGeometryCollection(geom)) {
		const GeometryCollection *gc=(GeometryCollection*) geom;
		for(int i=0;i<gc->getNumGeometries();i++) {
			addToList(gc->getGeometryN(i),geomList);
		}
	} else
		geomList->push_back(geom);
	return geomList;
}

bool RelateOp::isBaseGeometryCollection(const Geometry* geom) {
	//if ((typeid(*geom)==typeid(GeometryCollection)) ||
	//	(typeid(*geom)==typeid(MultiPoint)) ||
	//	(typeid(*geom)==typeid(MultiLineString)) ||
	//	(typeid(*geom)==typeid(MultiPolygon))) {
	//	return true;
	//} else {
	//	return false;
	//}
	if (typeid(*geom)==typeid(GeometryCollection)) {
		return true;
	} else {
		return false;
	}

}
}

