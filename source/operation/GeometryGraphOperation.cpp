/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.13  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../headers/operation.h"
#include "stdio.h"

namespace geos {

CGAlgorithms* GeometryGraphOperation::cga=new CGAlgorithms();
LineIntersector* GeometryGraphOperation::li=new RobustLineIntersector();

GeometryGraphOperation::GeometryGraphOperation(const Geometry *g0, const Geometry *g1) {
	// use the most precise model for the result
	if (g0->getPrecisionModel()->compareTo(g1->getPrecisionModel())>=0)
		setComputationPrecision(g0->getPrecisionModel());
	else
		setComputationPrecision(g1->getPrecisionModel());
	arg=new vector<GeometryGraph*>(2);
	(*arg)[0]=new GeometryGraph(0,g0);
	(*arg)[1]=new GeometryGraph(1, g1);
}


GeometryGraphOperation::GeometryGraphOperation(const Geometry *g0) {
	setComputationPrecision(g0->getPrecisionModel());
	arg=new vector<GeometryGraph*>(1);
	(*arg)[0]=new GeometryGraph(0,g0);;
}

const Geometry* GeometryGraphOperation::getArgGeometry(int i) const {
	return (*arg)[i]->getGeometry();
}

void GeometryGraphOperation::setComputationPrecision(const PrecisionModel* pm) {
    resultPrecisionModel=pm;
    li->setPrecisionModel(resultPrecisionModel);
}

GeometryGraphOperation::~GeometryGraphOperation() {
	//delete resultPrecisionModel;
	//delete arg;
	for(int i=0;i<(int)arg->size();i++) {
		delete (*arg)[i];
	}
	delete arg;
}

}

