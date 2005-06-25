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

#include <geos/opOverlay.h>
#include <geos/io.h>
#include <stdio.h>

#ifndef DEBUG
#define DEBUG 0
#endif
#ifndef COMPUTE_Z
#define COMPUTE_Z 0
#endif

namespace geos {

PointBuilder::PointBuilder(OverlayOp *newOp, const GeometryFactory *newGeometryFactory,PointLocator *newPtLocator)
{
	op=newOp;
	geometryFactory=newGeometryFactory;
	ptLocator=newPtLocator;
}

/*
 * @return a list of the Points in the result of the specified
 * overlay operation
 */
vector<Point*>*
PointBuilder::build(int opCode)
{
	vector<Node*>* nodeList=collectNodes(opCode);
	vector<Point*>* resultPointList=simplifyPoints(nodeList);
	delete nodeList;
	return resultPointList;
}

vector<Node*>*
PointBuilder::collectNodes(int opCode)
{
	vector<Node*>* resultNodeList=new vector<Node*>();
	// add nodes from edge intersections which have not already been
	// included in the result
	map<Coordinate*,Node*,CoordLT> &nodeMap=op->getGraph()->getNodeMap()->nodeMap;
	map<Coordinate*,Node*,CoordLT>::iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();it++)
	{
		Node *node=it->second;
		if (node->isInResult()) continue;
		if (node->isIncidentEdgeInResult()) continue;

		Label *label=node->getLabel();
		if (OverlayOp::isResultOfOp(label,opCode)) 
			resultNodeList->push_back(node);
	}
	return resultNodeList;
}

/*
 * This method simplifies the resultant Geometry by finding and eliminating
 * "covered" points.
 * A point is covered if it is contained in another element Geometry
 * with higher dimension (e.g. a point might be contained in a polygon,
 * in which case the point can be eliminated from the resultant).
 */
vector<Point*>*
PointBuilder::simplifyPoints(vector<Node*> *resultNodeList)
{
	vector<Point*>* nonCoveredPointList=new vector<Point*>();
	for(int i=0;i<(int)resultNodeList->size();i++)
	{
		Node *n=(*resultNodeList)[i];
		const Coordinate& coord=n->getCoordinate();
		if(!op->isCoveredByLA(coord)) {
			Point *pt=geometryFactory->createPoint(coord);
			nonCoveredPointList->push_back(pt);
		}
	}
	return nonCoveredPointList;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2005/06/25 10:20:39  strk
 * OverlayOp speedup (JTS port)
 *
 * Revision 1.14  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.13  2004/11/17 15:09:08  strk
 * Changed COMPUTE_Z defaults to be more conservative
 *
 * Revision 1.12  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.11  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.10  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.9  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
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

