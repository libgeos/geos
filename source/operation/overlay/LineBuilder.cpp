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

#define DEBUG 0
#define COMPUTE_Z 0

namespace geos {

LineBuilder::LineBuilder(OverlayOp *newOp, const GeometryFactory *newGeometryFactory, PointLocator *newPtLocator)
{
	op=newOp;
	geometryFactory=newGeometryFactory;
	ptLocator=newPtLocator;
	lineEdgesList=new vector<Edge*>();
	resultLineList=new vector<LineString*>();
}

LineBuilder::~LineBuilder()
{
	delete lineEdgesList;
}

/*
 * @return a list of the LineStrings in the result of the
 *         specified overlay operation
 */
vector<LineString*>*
LineBuilder::build(int opCode)
{
	findCoveredLineEdges();
	collectLines(opCode);
	//labelIsolatedLines(lineEdgesList);
	buildLines(opCode);
	return resultLineList;
}

/*
 * Find and mark L edges which are "covered" by the result area (if any).
 * L edges at nodes which also have A edges can be checked by checking
 * their depth at that node.
 * L edges at nodes which do not have A edges can be checked by doing a
 * point-in-polygon test with the previously computed result areas.
 */
void
LineBuilder::findCoveredLineEdges()
{
// first set covered for all L edges at nodes which have A edges too
	map<Coordinate,Node*,CoordLT> *nodeMap=op->getGraph()->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		//node.print(System.out);
		((DirectedEdgeStar*)node->getEdges())->findCoveredLineEdges();
	}

	/*
	 * For all L edges which weren't handled by the above,
	 * use a point-in-poly test to determine whether they are covered
	 */
	vector<EdgeEnd*> *ee=op->getGraph()->getEdgeEnds();
	for(int i=0;i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*) (*ee)[i];
		Edge *e=de->getEdge();
		if (de->isLineEdge() && !e->isCoveredSet()) {
			bool isCovered=op->isCoveredByA(de->getCoordinate());
			e->setCovered(isCovered);
		}
	}
}

void
LineBuilder::collectLines(int opCode)
{
	vector<EdgeEnd*> *ee=op->getGraph()->getEdgeEnds();
	for(int i=0;i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*) (*ee)[i];
		collectLineEdge(de,opCode,lineEdgesList);
		collectBoundaryTouchEdge(de,opCode,lineEdgesList);
	}
}

void
LineBuilder::collectLineEdge(DirectedEdge *de,int opCode,vector<Edge*> *edges)
{
	Label *label=de->getLabel();
	Edge *e=de->getEdge();
	// include L edges which are in the result
	if (de->isLineEdge()) {
		if (!de->isVisited() && OverlayOp::isResultOfOp(label,opCode) && !e->isCovered()) {
			//Debug.println("de: "+de.getLabel());
			//Debug.println("edge: "+e.getLabel());
			edges->push_back(e);
			de->setVisitedEdge(true);
		}
	}
}

/*
 * Collect edges from Area inputs which should be in the result but
 * which have not been included in a result area.
 * This happens ONLY:
 *  - during an intersection when the boundaries of two
 *    areas touch in a line segment
 *  - OR as a result of a dimensional collapse.
 */
void
LineBuilder::collectBoundaryTouchEdge(DirectedEdge *de,int opCode,vector<Edge*> *edges)
{
	Label *label=de->getLabel();
	// this smells like a bit of a hack, but it seems to work...
	if (!de->isLineEdge()
		&& !de->isInteriorAreaEdge()  // added to handle dimensional collapses
		&& !de->getEdge()->isInResult()
		&& !de->isVisited()
		&& OverlayOp::isResultOfOp(label,opCode)
		&& opCode==OverlayOp::INTERSECTION) {
			edges->push_back(de->getEdge());
			de->setVisitedEdge(true);
	}
}

void
LineBuilder::buildLines(int opCode)
{
	// need to simplify lines?
	for(int i=0;i<(int)lineEdgesList->size();i++) {
		Edge *e=(*lineEdgesList)[i];
		//Label *label=e->getLabel();
		CoordinateSequence *cs = e->getCoordinates()->clone();
#if COMPUTE_Z
		propagateZ(cs);
#endif
		LineString *line=geometryFactory->createLineString(cs);
		resultLineList->push_back(line);
		e->setInResult(true);
	}
}

/*
 * If the given CoordinateSequence has mixed 3d/2d vertexes
 * set Z for all vertexes missing it.
 * The Z value is interpolated between 3d vertexes and copied
 * from a 3d vertex to the end.
 */
void
LineBuilder::propagateZ(CoordinateSequence *cs)
{
	unsigned int i;
#if DEBUG
	cerr<<"LineBuilder::propagateZ() called"<<endl;
#endif

	vector<int>v3d; // vertex 3d
	unsigned int cssize = cs->getSize();
	for (i=0; i<cssize; i++)
	{
		if ( cs->getAt(i).z != DoubleNotANumber ) v3d.push_back(i);
	}

#if DEBUG
	cerr<<"  found "<<v3d.size()<<" 3d vertexes"<<endl;
#endif
	
	if ( v3d.size() == 0 )
	{
#if DEBUG
		cerr<<"  nothing to do"<<endl;
#endif
		return;
	}

	Coordinate buf;

	// fill initial part
	if ( v3d[0] != 0 )
	{
		double z = cs->getAt(v3d[0]).z;
		for (int j=0; j<v3d[0]; j++)
		{
			buf = cs->getAt(j);
			buf.z = z;
			cs->setAt(buf, j);
		}
	}

	// interpolate inbetweens
	int prev=v3d[0];
	for (i=1; i<v3d.size(); i++)
	{
		int curr=v3d[i];
		int dist = curr-prev;
		if (dist > 1)
		{
			const Coordinate &cto = cs->getAt(curr);
			const Coordinate &cfrom = cs->getAt(prev);
			double gap = cto.z-cfrom.z;
			double zstep = gap/dist;
			double z = cfrom.z;
			for (int j=prev+1; j<curr; j++)
			{
				buf = cs->getAt(j);
				z+=zstep;
				buf.z = z;
				cs->setAt(buf, j);
			}
		}
		prev = curr;
	}

	// fill final part
	if ( prev < cssize-1 )
	{
		double z = cs->getAt(prev).z;
		for (int j=prev+1; j<cssize; j++)
		{
			buf = cs->getAt(j);
			buf.z = z;
			cs->setAt(buf, j);
		}
	}

}



void
LineBuilder::labelIsolatedLines(vector<Edge*> *edgesList)
{
	for(int i=0;i<(int)edgesList->size();i++) {
		Edge *e=(*edgesList)[i];
		Label *label=e->getLabel();
		//n.print(System.out);
		if (e->isIsolated()) {
			if (label->isNull(0))
				labelIsolatedLine(e,0);
			else
				labelIsolatedLine(e,1);
		}
	}
}

/*
 * Label an isolated node with its relationship to the target geometry.
 */
void
LineBuilder::labelIsolatedLine(Edge *e,int targetIndex)
{
	int loc=ptLocator->locate(e->getCoordinate(),op->getArgGeometry(targetIndex));
	e->getLabel()->setLocation(targetIndex,loc);
}

}

/**********************************************************************
 * $Log$
 * Revision 1.15  2004/11/23 19:53:07  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.14  2004/11/20 18:17:26  strk
 * Added Z propagation for overlay lines output.
 *
 * Revision 1.13  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.12  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.11  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.10  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.8  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more calls 
 * to new getCoordinatesRO() when applicable.
 *
 **********************************************************************/

