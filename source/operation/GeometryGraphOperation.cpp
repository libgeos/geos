#include "../headers/operation.h"
#include "stdio.h"

CGAlgorithms* GeometryGraphOperation::cga=new RobustCGAlgorithms();
LineIntersector* GeometryGraphOperation::li=new RobustLineIntersector();

GeometryGraphOperation::GeometryGraphOperation(Geometry *g0,Geometry *g1) {
	// use the most precise model for the result
	if (g0->getPrecisionModel()->compareTo(g1->getPrecisionModel())>=0)
		setComputationPrecision(g0->getPrecisionModel());
	else
		setComputationPrecision(g1->getPrecisionModel());
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

void GeometryGraphOperation::setComputationPrecision(PrecisionModel* pm) {
    resultPrecisionModel=pm;
    li->setMakePrecise(resultPrecisionModel);
}

