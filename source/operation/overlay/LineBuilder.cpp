/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <map>
#include <vector>
#include <cassert>

#include <geos/operation/overlay/LineBuilder.h>
#include <geos/operation/overlay/OverlayOp.h>

#include <geos/algorithm/PointLocator.h>

#include <geos/geom/GeometryFactory.h>

#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/DirectedEdgeStar.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif
#define COMPUTE_Z 1

using namespace std;
using namespace geos::algorithm;
using namespace geos::geomgraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

LineBuilder::LineBuilder(OverlayOp *newOp,
		const GeometryFactory *newGeometryFactory,
		PointLocator *newPtLocator):
	op(newOp),
	geometryFactory(newGeometryFactory),
	ptLocator(newPtLocator),
	//lineEdgesList(new vector<Edge *>()),
	resultLineList(new vector<LineString *>())
{
}

LineBuilder::~LineBuilder()
{
}

/*
 * @return a list of the LineStrings in the result of the
 *         specified overlay operation
 */
vector<LineString*>*
LineBuilder::build(OverlayOp::OpCode opCode)
{
	findCoveredLineEdges();
	collectLines(opCode);
	//labelIsolatedLines(&lineEdgesList);
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
	map<Coordinate*,Node*,CoordinateLessThen> &nodeMap=op->getGraph().getNodeMap()->nodeMap;
	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	map<Coordinate*,Node*,CoordinateLessThen>::iterator endIt=nodeMap.end();
	for ( ; it!=endIt; ++it)
	{
		Node *node=it->second;
		//node.print(System.out);
		assert(dynamic_cast<DirectedEdgeStar*>(node->getEdges()));
		DirectedEdgeStar* des=static_cast<DirectedEdgeStar*>(node->getEdges());
		des->findCoveredLineEdges();
		//((DirectedEdgeStar*)node->getEdges())->findCoveredLineEdges();
	}

	/*
	 * For all L edges which weren't handled by the above,
	 * use a point-in-poly test to determine whether they are covered
	 */
	vector<EdgeEnd*> *ee=op->getGraph().getEdgeEnds();
	for(size_t i=0, s=ee->size(); i<s; ++i)
	{
		assert(dynamic_cast<DirectedEdge*>((*ee)[i]));
		DirectedEdge *de=static_cast<DirectedEdge*>((*ee)[i]);
		Edge *e=de->getEdge();
		if (de->isLineEdge() && !e->isCoveredSet()) {
			bool isCovered=op->isCoveredByA(de->getCoordinate());
			e->setCovered(isCovered);
		}
	}
}

void
LineBuilder::collectLines(OverlayOp::OpCode opCode)
{
	vector<EdgeEnd*> *ee=op->getGraph().getEdgeEnds();
	for(size_t i=0, s=ee->size(); i<s; ++i)
	{
		assert(dynamic_cast<DirectedEdge*>((*ee)[i]));
		DirectedEdge *de=static_cast<DirectedEdge*>((*ee)[i]);
		collectLineEdge(de, opCode, &lineEdgesList);
		collectBoundaryTouchEdge(de, opCode, &lineEdgesList);
	}
}

void
LineBuilder::collectLineEdge(DirectedEdge *de, OverlayOp::OpCode opCode,
		vector<Edge*> *edges)
{
	Label *label=de->getLabel();
	Edge *e=de->getEdge();
	// include L edges which are in the result
	if (de->isLineEdge()) {
		if (!de->isVisited()
			&& OverlayOp::isResultOfOp(label,opCode)
			&& !e->isCovered())
		{
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
LineBuilder::collectBoundaryTouchEdge(DirectedEdge *de,
		OverlayOp::OpCode opCode, vector<Edge*> *edges)
{
	Label *label=de->getLabel();
	// this smells like a bit of a hack, but it seems to work...
	if (!de->isLineEdge()
		&& !de->isInteriorAreaEdge()  // added to handle dimensional collapses
		&& !de->getEdge()->isInResult()
		&& !de->isVisited()
		&& OverlayOp::isResultOfOp(label, opCode)
		&& opCode==OverlayOp::opINTERSECTION)
	{
			edges->push_back(de->getEdge());
			de->setVisitedEdge(true);
	}
}

void
LineBuilder::buildLines(OverlayOp::OpCode /* opCode */)
{
	// need to simplify lines?
	for(size_t i=0, s=lineEdgesList.size(); i<s; ++i)
	{
		Edge *e=lineEdgesList[i];
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
	size_t i;
#if GEOS_DEBUG
	cerr<<"LineBuilder::propagateZ() called"<<endl;
#endif

	vector<int>v3d; // vertex 3d
	size_t cssize = cs->getSize();
	for (i=0; i<cssize; i++)
	{
		if ( !ISNAN(cs->getAt(i).z) ) v3d.push_back(i);
	}

#if GEOS_DEBUG
	cerr<<"  found "<<v3d.size()<<" 3d vertexes"<<endl;
#endif
	
	if ( v3d.size() == 0 )
	{
#if GEOS_DEBUG
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
	size_t prev=v3d[0];
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
		for (size_t j=prev+1; j<cssize; j++)
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
	for(size_t i=0, s=edgesList->size(); i<s; ++i)
	{
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
LineBuilder::labelIsolatedLine(Edge *e, int targetIndex)
{
	int loc=ptLocator->locate(e->getCoordinate(),op->getArgGeometry(targetIndex));
	e->getLabel()->setLocation(targetIndex,loc);
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.28  2006/06/13 22:19:39  strk
 * fixed unused parameter warning
 *
 * Revision 1.27  2006/06/12 11:29:24  strk
 * unsigned int => size_t
 *
 * Revision 1.26  2006/06/05 15:36:34  strk
 * Given OverlayOp funx code enum a name and renamed values to have a lowercase prefix. Drop all of noding headers from installed header set.
 *
 * Revision 1.25  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.24  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.23  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.22  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.21  2005/12/08 00:03:51  strk
 * LineBuilder::lineEdgesList made a real vector, rather then pointer (private member).
 * Small optimizations in LineBuilder loops, cleanups in LineBuilder class dox.
 *
 * Revision 1.20  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.19  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.18  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.17  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.16  2004/11/24 11:32:39  strk
 * Re-enabled Z propagation in output lines.
 *
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

