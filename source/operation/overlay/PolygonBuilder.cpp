/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/overlay/PolygonBuilder.java rev. 1.18 (JTS-1.7)
 *
 **********************************************************************/

#include <cassert>

#include <geos/opOverlay.h>
#include <geos/algorithm/CGAlgorithms.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 1
#endif

using namespace std;
using namespace geos::geomgraph;
using namespace geos::algorithm;


namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

PolygonBuilder::PolygonBuilder(const GeometryFactory *newGeometryFactory)
	:
	geometryFactory(newGeometryFactory)
{
}

PolygonBuilder::~PolygonBuilder()
{
	for(unsigned int i=0; i<shellList.size(); ++i)
	{
		delete shellList[i];
	}
}

/*public*/
void
PolygonBuilder::add(PlanarGraph *graph)
	//throw(TopologyException *)
{
	vector<EdgeEnd*>* eeptr=graph->getEdgeEnds();
	assert(eeptr);
	vector<EdgeEnd*>& ee = *eeptr;

	unsigned int eeSize=ee.size();
	vector<DirectedEdge*> dirEdges(eeSize);

	for(unsigned int i=0; i<eeSize; ++i)
	{
		DirectedEdge* de = dynamic_cast<DirectedEdge*>(ee[i]);
		assert(de);
		dirEdges[i]=de;
	}

	map<Coordinate*,Node*,CoordinateLessThen> &nodeMap=graph->getNodeMap()->nodeMap;
	vector<Node*> nodes;
	nodes.reserve(nodeMap.size());

	map<Coordinate*,Node*,CoordinateLessThen>::iterator it=nodeMap.begin();
	for (;it!=nodeMap.end();++it) {
		Node *node=it->second;
		nodes.push_back(node);
	}

	add(&dirEdges, &nodes); // might throw a TopologyException *
}

/*public*/
void
PolygonBuilder::add(vector<DirectedEdge*> *dirEdges, vector<Node*> *nodes)
	//throw(TopologyException *)
{
	//	PlanarGraph::linkResultDirectedEdgesS(nodes);

	vector<Node*>::iterator nodeit=nodes->begin();
	for(;nodeit!=nodes->end();++nodeit) {
		Node *node=*nodeit;
		DirectedEdgeStar* des = dynamic_cast<DirectedEdgeStar*>(node->getEdges());
		assert(des);

		// This might throw a TopologyException
		des->linkResultDirectedEdges();
	}

	vector<MaximalEdgeRing*>* maxEdgeRings=buildMaximalEdgeRings(dirEdges);
	vector<EdgeRing*> freeHoleList;
	vector<MaximalEdgeRing*> *edgeRings=buildMinimalEdgeRings(maxEdgeRings,&shellList,&freeHoleList);
	sortShellsAndHoles(edgeRings,&shellList,&freeHoleList);
	placeFreeHoles(&shellList, &freeHoleList);
	delete maxEdgeRings;
	delete edgeRings;
	//Assert: every hole on freeHoleList has a shell assigned to it
}

/*public*/
vector<Geometry*>*
PolygonBuilder::getPolygons()
{
	vector<Geometry*> *resultPolyList=computePolygons(&shellList);
	return resultPolyList;
}


/*private*/
vector<MaximalEdgeRing*> *
PolygonBuilder::buildMaximalEdgeRings(vector<DirectedEdge*> *dirEdges)
{
#if GEOS_DEBUG
	cerr<<"PolygonBuilder::buildMaximalEdgeRings got "<<dirEdges->size()<<" dirEdges"<<endl;
#endif
	vector<MaximalEdgeRing*> *maxEdgeRings=new vector<MaximalEdgeRing*>();
	for(unsigned int i=0, n=dirEdges->size(); i<n; i++)
	{
		DirectedEdge *de=(*dirEdges)[i];
#if GEOS_DEBUG
	cerr << "  dirEdge " << i << endl
	     << de->printEdge() << endl
	     << " inResult:" << de->isInResult()
	     << " isArea:" << de->getLabel()->isArea() << endl;
#endif
		if (de->isInResult() && de->getLabel()->isArea()) {
			// if this edge has not yet been processed
			if (de->getEdgeRing()==NULL) {
				MaximalEdgeRing *er=new MaximalEdgeRing(de,geometryFactory);
				maxEdgeRings->push_back(er);
				er->setInResult();
				//System.out.println("max node degree=" + er.getMaxDegree());
			}
		}
	}
#if GEOS_DEBUG
	cerr<<"  returning "<<maxEdgeRings->size()<<" maxEdgeRings"<<endl;
#endif
	return maxEdgeRings;
}

/*private*/
vector<MaximalEdgeRing*> *
PolygonBuilder::buildMinimalEdgeRings(vector<MaximalEdgeRing*> *maxEdgeRings,
	vector<EdgeRing*> *newShellList, vector<EdgeRing*> *freeHoleList)
{
	vector<MaximalEdgeRing*> *edgeRings=new vector<MaximalEdgeRing*>();
	for(int i=0;i<(int)maxEdgeRings->size();i++) {
		MaximalEdgeRing *er=(*maxEdgeRings)[i];
#if GEOS_DEBUG
	cerr<<"buildMinimalEdgeRings: maxEdgeRing "<<i<<" has "<<er->getMaxNodeDegree()<<" maxNodeDegree"<<endl;
#endif
		if (er->getMaxNodeDegree()>2) {
			er->linkDirectedEdgesForMinimalEdgeRings();
			vector<MinimalEdgeRing*> *minEdgeRings=er->buildMinimalRings();
			// at this point we can go ahead and attempt to place
			// holes, if this EdgeRing is a polygon
			//computePoints(minEdgeRings);
			EdgeRing *shell=findShell(minEdgeRings);
			if(shell!=NULL){
				placePolygonHoles(shell,minEdgeRings);
				newShellList->push_back(shell);
			} else {
				freeHoleList->insert(freeHoleList->end(),minEdgeRings->begin(),minEdgeRings->end());
			}
			delete er;
			delete minEdgeRings;
		} else {
			edgeRings->push_back(er);
		}
	}
	return edgeRings;
}

/*private*/
EdgeRing*
PolygonBuilder::findShell(vector<MinimalEdgeRing*> *minEdgeRings)
{
	int shellCount=0;
	EdgeRing *shell=NULL;
#if GEOS_DEBUG
	cerr<<"PolygonBuilder::findShell got "<<minEdgeRings->size()<<" minEdgeRings"<<endl;
#endif
	for(int i=0;i<(int)minEdgeRings->size();i++) {
		EdgeRing *er=(*minEdgeRings)[i];
		if (!er->isHole()) {
			shell=er;
			shellCount++;
			// Should MinimalEdgeRing object pointed to
			minEdgeRings->erase(minEdgeRings->begin()+i);
			i--;
		}
	}
	assert(shellCount <= 1); // found two shells in MinimalEdgeRing list
	return shell;
}

/*private*/
void
PolygonBuilder::placePolygonHoles(EdgeRing *shell,
	vector<MinimalEdgeRing*> *minEdgeRings)
{
	for(int i=0;i<(int)minEdgeRings->size();i++) {
		MinimalEdgeRing *er=(*minEdgeRings)[i];
		if (er->isHole()) {
			er->setShell(shell);
			minEdgeRings->erase(minEdgeRings->begin()+i);
			i--;
		}
	}
}

/*private*/
void
PolygonBuilder::sortShellsAndHoles(vector<MaximalEdgeRing*> *edgeRings,
	vector<EdgeRing*> *newShellList, vector<EdgeRing*> *freeHoleList)
{
	for(unsigned int i=0, n=edgeRings->size(); i<n; i++)
	{
		EdgeRing *er=(*edgeRings)[i];
		//er->setInResult();
		if (er->isHole() ) {
			freeHoleList->push_back(er);
		} else {
			newShellList->push_back(er);
		}
	}
}

/*private*/
void
PolygonBuilder::placeFreeHoles(std::vector<EdgeRing*>* newShellList,
	std::vector<EdgeRing*> *freeHoleList)
{
	for(std::vector<EdgeRing*>::iterator
			it=freeHoleList->begin(), itEnd=freeHoleList->end();
			it != itEnd;
			++it)
	{
		EdgeRing *hole=*it;
		// only place this hole if it doesn't yet have a shell
		if (hole->getShell()==NULL) {
			EdgeRing *shell=findEdgeRingContaining(hole, newShellList);
			assert(shell!=NULL); // unable to assign hole to a shell
			hole->setShell(shell);
		}
	}
}

/*private*/
EdgeRing*
PolygonBuilder::findEdgeRingContaining(EdgeRing *testEr,
	vector<EdgeRing*> *newShellList)
{
	LinearRing *testRing=testEr->getLinearRing();
	const Envelope *testEnv=testRing->getEnvelopeInternal();
	const Coordinate& testPt=testRing->getCoordinateN(0);
	EdgeRing *minShell=NULL;
	const Envelope *minEnv=NULL;
	for(int i=0;i<(int)newShellList->size();i++) {
		LinearRing *lr=NULL;
		EdgeRing *tryShell=(*newShellList)[i];
		LinearRing *tryRing=tryShell->getLinearRing();
		const Envelope *tryEnv=tryRing->getEnvelopeInternal();
		if (minShell!=NULL) {
			lr=minShell->getLinearRing();
			minEnv=lr->getEnvelopeInternal();
		}
		bool isContained=false;
		const CoordinateSequence *rcl = tryRing->getCoordinatesRO();
		if (tryEnv->contains(testEnv)
			&& CGAlgorithms::isPointInRing(testPt,rcl))
				isContained=true;
		// check if this new containing ring is smaller than
		// the current minimum ring
		if (isContained) {
			if (minShell==NULL
				|| minEnv->contains(tryEnv)) {
					minShell=tryShell;
			}
		}
	}
	return minShell;
}

/*private*/
vector<Geometry*>*
PolygonBuilder::computePolygons(vector<EdgeRing*> *newShellList)
{
#if GEOS_DEBUG
	cerr<<"PolygonBuilder::computePolygons: got "<<newShellList->size()<<" shells"<<endl;
#endif
	vector<Geometry*> *resultPolyList=new vector<Geometry*>();
	// add Polygons for all shells
	for(int i=0;i<(int)newShellList->size();i++) {
		EdgeRing *er=(*newShellList)[i];
		Polygon *poly=er->toPolygon(geometryFactory);
		resultPolyList->push_back(poly);
	}
	return resultPolyList;
}

/*public*/
bool
PolygonBuilder::containsPoint(const Coordinate& p)
{
	for(unsigned int i=0;i<shellList.size(); ++i) {
		EdgeRing *er=shellList[i];
		if (er->containsPoint(p))
			return true;
	}
	return false;
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.33  2006/03/14 16:55:01  strk
 * comments cleanup
 *
 * Revision 1.32  2006/03/14 14:16:52  strk
 * operator<< for BufferSubgraph, more debugging calls
 *
 * Revision 1.31  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.30  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.29  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.28  2006/03/02 14:34:43  strk
 * GeometryGraphOperation::li made a non-static member, and not more a pointer
 *
 * Revision 1.27  2006/03/02 12:12:01  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.26  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.25  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.24  2006/01/20 19:28:39  strk
 * Updated port information
 *
 * Revision 1.23  2006/01/20 19:21:24  strk
 * Fixed bug #13
 *
 * Revision 1.22  2005/11/18 00:55:29  strk
 *
 * Fixed a bug in EdgeRing::containsPoint().
 * Changed EdgeRing::getLinearRing() to avoid LinearRing copy and updated
 * usages from PolygonBuilder.
 * Removed CoordinateSequence copy in EdgeRing (ownership is transferred
 * to its LinearRing).
 * Removed heap allocations for EdgeRing containers.
 * Initialization lists and cleanups.
 *
 * Revision 1.21  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.20  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.19  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.18  2004/07/27 16:35:47  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.17  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.16  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.15  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.14  2004/05/03 10:43:43  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.13  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/12 18:02:57  strk
 * Added throw specification. Fixed leaks on exceptions.
 *
 * Revision 1.11  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.10  2003/11/06 18:48:30  strk
 * added throw information comment in PolygonBuilder
 *
 * Revision 1.9  2003/10/13 17:47:49  strk
 * delete statement removed
 *
 **********************************************************************/

