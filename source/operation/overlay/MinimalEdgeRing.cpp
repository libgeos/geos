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
 * Revision 1.8  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/opOverlay.h"
#include "stdio.h"

namespace geos {

MinimalEdgeRing::MinimalEdgeRing(DirectedEdge *start, const GeometryFactory *geometryFactory,CGAlgorithms *cga):
	EdgeRing(start,geometryFactory,cga){
	computePoints(start);
	computeRing();
}

MinimalEdgeRing::~MinimalEdgeRing(){
}

DirectedEdge* MinimalEdgeRing::getNext(DirectedEdge *de) {
	return de->getNextMin();
}

void MinimalEdgeRing::setEdgeRing(DirectedEdge *de,EdgeRing *er) {
	de->setMinEdgeRing(er);
}
}

