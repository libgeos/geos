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
 **********************************************************************/

#include <geos/operation.h>
#include <stdio.h>

namespace geos {

CGAlgorithms* GeometryGraphOperation::cga=new CGAlgorithms();
LineIntersector* GeometryGraphOperation::li=new LineIntersector();

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

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.17  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.16  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.15  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.14  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
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

