#include "operation.h"
#include "stdio.h"

CGAlgorithms* GeometryGraphOperation::cga=new RobustCGAlgorithms();
LineIntersector* GeometryGraphOperation::li=new RobustLineIntersector();

GeometryGraphOperation::GeometryGraphOperation(Geometry *g0,Geometry *g1) {
	setComputationPrecision(g0->getPrecisionModel());
	arg=new vector<GeometryGraph*>(2);
	(*arg)[0]=new GeometryGraph(0,g0);
	(*arg)[1]=new GeometryGraph(1, g1);
}


GeometryGraphOperation::GeometryGraphOperation(Geometry *g0) {
	setComputationPrecision(g0->getPrecisionModel());
	arg=new vector<GeometryGraph*>(1);
	(*arg)[0]=new GeometryGraph(0,g0);;
}

Geometry* GeometryGraphOperation::getArgGeometry(int i) {
	return (*arg)[i]->getGeometry();
}

void GeometryGraphOperation::setComputationPrecision(PrecisionModel pm) {
	makePrecise=!pm.isFloating();
	li->setMakePrecise(makePrecise);
}