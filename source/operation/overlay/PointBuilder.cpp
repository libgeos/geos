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
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/opOverlay.h"
#include "stdio.h"

namespace geos {

PointBuilder::PointBuilder(OverlayOp *newOp,GeometryFactory *newGeometryFactory,PointLocator *newPtLocator) {
	op=newOp;
	geometryFactory=newGeometryFactory;
	ptLocator=newPtLocator;
}

/**
* @return a list of the Points in the result of the specified overlay operation
*/
vector<Point*>* PointBuilder::build(int opCode){
	vector<Node*>* nodeList=collectNodes(opCode);
	vector<Point*>* resultPointList=simplifyPoints(nodeList);
	delete nodeList;
	return resultPointList;
}

vector<Node*>* PointBuilder::collectNodes(int opCode){
	vector<Node*>* resultNodeList=new vector<Node*>();
	// add nodes from edge intersections which have not already been included in the result
	map<Coordinate,Node*,CoordLT> *nodeMap=op->getGraph()->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		if (!node->isInResult()) {
			Label *label=node->getLabel();
			if (OverlayOp::isResultOfOp(label,opCode)) {
				resultNodeList->push_back(node);
			}
		}
	}
	return resultNodeList;
}
/**
* This method simplifies the resultant Geometry by finding and eliminating
* "covered" points.
* A point is covered if it is contained in another element Geometry
* with higher dimension (e.g. a point might be contained in a polygon,
* in which case the point can be eliminated from the resultant).
*/
vector<Point*>* PointBuilder::simplifyPoints(vector<Node*> *resultNodeList) {
	vector<Point*>* nonCoveredPointList=new vector<Point*>();
	for(int i=0;i<(int)resultNodeList->size();i++) {
		Node *n=(*resultNodeList)[i];
		const Coordinate& coord=n->getCoordinate();
		if(!op->isCoveredByLA(coord)) {
			Point *pt=geometryFactory->createPoint(coord);
			nonCoveredPointList->push_back(pt);
		}
	}
	return nonCoveredPointList;
}
}

