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
 * Revision 1.18  2004/07/27 16:35:47  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.17  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
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


#include <geos/opOverlay.h>
#include <stdio.h>
#include <geos/util.h>

namespace geos {

PolygonBuilder::PolygonBuilder(const GeometryFactory *newGeometryFactory, CGAlgorithms *newCga) {
	geometryFactory=newGeometryFactory;
	cga=newCga;
	shellList=new vector<EdgeRing*>();
}

PolygonBuilder::~PolygonBuilder() {
	for(int i=0;i<(int)shellList->size();i++) {
		delete (*shellList)[i];
	}
	delete shellList;
}

void
PolygonBuilder::add(PlanarGraph *graph)
	//throw(TopologyException *)
{
	vector<DirectedEdge*> *dirEdges=new vector<DirectedEdge*>();
	vector<Node*> *nodes=new vector<Node*>();
	vector<EdgeEnd*> *ee=graph->getEdgeEnds();
	for(int i=0;i<(int)ee->size();i++) {
		dirEdges->push_back((DirectedEdge*)(*ee)[i]);
	}
	map<Coordinate,Node*,CoordLT> *nodeMap=graph->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		nodes->push_back(node);
	}
	try {
		add(dirEdges,nodes); // might throw a TopologyException *
	} catch (...) {
		delete dirEdges;
		delete nodes;
		throw;
	}
	delete dirEdges;
	delete nodes;
}

void
PolygonBuilder::add(vector<DirectedEdge*> *dirEdges,vector<Node*> *nodes)
	//throw(TopologyException *)
{
	//	PlanarGraph::linkResultDirectedEdgesS(nodes);

	for(vector<Node*>::iterator nodeit=nodes->begin();nodeit<nodes->end();nodeit++) {
		Node *node=*nodeit;
		// This might throw a TopologyException
		((DirectedEdgeStar*) node->getEdges())->linkResultDirectedEdges();
	}

	vector<MaximalEdgeRing*>* maxEdgeRings=buildMaximalEdgeRings(dirEdges);
	vector<EdgeRing*> *freeHoleList=new vector<EdgeRing*>();
	vector<MaximalEdgeRing*> *edgeRings=buildMinimalEdgeRings(maxEdgeRings,shellList,freeHoleList);
	sortShellsAndHoles(edgeRings,shellList,freeHoleList);
	placeFreeHoles(shellList, freeHoleList);
	delete freeHoleList;
	delete maxEdgeRings;
	delete edgeRings;
	//Assert: every hole on freeHoleList has a shell assigned to it
}

vector<Geometry*>* PolygonBuilder::getPolygons() {
	vector<Geometry*> *resultPolyList=computePolygons(shellList);
	return resultPolyList;
}


/**
* for all DirectedEdges in result, form them into MaximalEdgeRings
*/
vector<MaximalEdgeRing*> *PolygonBuilder::buildMaximalEdgeRings(vector<DirectedEdge*> *dirEdges){
	vector<MaximalEdgeRing*> *maxEdgeRings=new vector<MaximalEdgeRing*>();
	for(int i=0;i<(int)dirEdges->size();i++) {
		DirectedEdge *de=(*dirEdges)[i];
		if (de->isInResult() && de->getLabel()->isArea()) {
			// if this edge has not yet been processed
			if (de->getEdgeRing()==NULL) {
				MaximalEdgeRing *er=new MaximalEdgeRing(de,geometryFactory,cga);
				maxEdgeRings->push_back(er);
				//System.out.println("max node degree=" + er.getMaxDegree());
			}
		}
	}
	return maxEdgeRings;
}

vector<MaximalEdgeRing*> *
PolygonBuilder::buildMinimalEdgeRings(vector<MaximalEdgeRing*> *maxEdgeRings,
	vector<EdgeRing*> *newShellList, vector<EdgeRing*> *freeHoleList) {
	vector<MaximalEdgeRing*> *edgeRings=new vector<MaximalEdgeRing*>();
	for(int i=0;i<(int)maxEdgeRings->size();i++) {
		MaximalEdgeRing *er=(*maxEdgeRings)[i];
		if (er->getMaxNodeDegree()>2) {
			er->linkDirectedEdgesForMinimalEdgeRings();
			vector<MinimalEdgeRing*> *minEdgeRings=er->buildMinimalRings();
			// at this point we can go ahead and attempt to place holes, if this EdgeRing is a polygon
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

/**
* This method takes a list of MinimalEdgeRings derived from a MaximalEdgeRing,
* and tests whether they form a Polygon.  This is the case if there is a single shell
* in the list.  In this case the shell is returned.
* The other possibility is that they are a series of connected holes, in which case
* no shell is returned.
*
* @return the shell EdgeRing, if there is one
* @return null, if all the rings are holes
*/
EdgeRing* PolygonBuilder::findShell(vector<MinimalEdgeRing*> *minEdgeRings) {
	int shellCount=0;
	EdgeRing *shell=NULL;
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
	Assert::isTrue(shellCount <= 1, "found two shells in MinimalEdgeRing list");
	return shell;
}

/**
* This method assigns the holes for a Polygon (formed from a list of
* MinimalEdgeRings) to its shell.
* Determining the holes for a MinimalEdgeRing polygon serves two purposes:
* <ul>
* <li>it is faster than using a point-in-polygon check later on.
* <li>it ensures correctness, since if the PIP test was used the point
* chosen might lie on the shell, which might return an incorrect result from the
* PIP test
* </ul>
*/
void PolygonBuilder::placePolygonHoles(EdgeRing *shell,vector<MinimalEdgeRing*> *minEdgeRings) {
	for(int i=0;i<(int)minEdgeRings->size();i++) {
		MinimalEdgeRing *er=(*minEdgeRings)[i];
		if (er->isHole()) {
			er->setShell(shell);
			minEdgeRings->erase(minEdgeRings->begin()+i);
			i--;
		}
	}
}

/**
* For all rings in the input list,
* determine whether the ring is a shell or a hole
* and add it to the appropriate list.
* Due to the way the DirectedEdges were linked,
* a ring is a shell if it is oriented CW, a hole otherwise.
*/
void PolygonBuilder::sortShellsAndHoles(vector<MaximalEdgeRing*> *edgeRings,
												vector<EdgeRing*> *newShellList,
												vector<EdgeRing*> *freeHoleList) {
	for(int i=0;i<(int)edgeRings->size();i++) {
		EdgeRing *er=(*edgeRings)[i];
		er->setInResult();
		if (er->isHole() ) {
			freeHoleList->push_back(er);
		} else {
			newShellList->push_back(er);
		}
	}
}

/**
* This method determines finds a containing shell for all holes
* which have not yet been assigned to a shell.
* These "free" holes should
* all be <b>properly</b> contained in their parent shells, so it is safe to use the
* <code>findEdgeRingContaining</code> method.
* (This is the case because any holes which are NOT
* properly contained (i.e. are connected to their
* parent shell) would have formed part of a MaximalEdgeRing
* and been handled in a previous step).
*/
void PolygonBuilder::placeFreeHoles(vector<EdgeRing*>* newShellList, vector<EdgeRing*> *freeHoleList) {
	for(int i=0;i<(int)freeHoleList->size();i++) {
		EdgeRing *hole=(*freeHoleList)[i];
		// only place this hole if it doesn't yet have a shell
		if (hole->getShell()==NULL) {
			EdgeRing *shell=findEdgeRingContaining(hole,newShellList);
			Assert::isTrue(shell!=NULL, "unable to assign hole to a shell");
			hole->setShell(shell);
		}
	}
}

/**
* Find the innermost enclosing shell EdgeRing containing the argument EdgeRing, if any.
* The innermost enclosing ring is the <i>smallest</i> enclosing ring.
* The algorithm used depends on the fact that:
* <br>
*  ring A contains ring B iff envelope(ring A) contains envelope(ring B)
* <br>
* This routine is only safe to use if the chosen point of the hole
* is known to be properly contained in a shell
* (which is guaranteed to be the case if the hole does not touch its shell)
*
* @return containing EdgeRing, if there is one
* @return null if no containing EdgeRing is found
*/
EdgeRing* PolygonBuilder::findEdgeRingContaining(EdgeRing *testEr,vector<EdgeRing*> *newShellList) {
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
		CoordinateSequence *rcl = tryRing->getCoordinates();
		if (tryEnv->contains(testEnv)
			&& cga->isPointInRing(testPt,rcl))
				isContained=true;
		delete rcl;
		// check if this new containing ring is smaller than the current minimum ring
		if (isContained) {
			if (minShell==NULL
				|| minEnv->contains(tryEnv)) {
					minShell=tryShell;
			}
		}
		delete tryRing;
		delete lr;
		//delete tryEnv;
	}
	//delete minEnv;
	delete testRing;
	//delete testEnv;
	return minShell;
}
vector<Geometry*>* PolygonBuilder::computePolygons(vector<EdgeRing*> *newShellList) {
	vector<Geometry*> *resultPolyList=new vector<Geometry*>();
	// add Polygons for all shells
	for(int i=0;i<(int)newShellList->size();i++) {
		EdgeRing *er=(*newShellList)[i];
		Polygon *poly=er->toPolygon(geometryFactory);
		resultPolyList->push_back(poly);
	}
	return resultPolyList;
}

/**
* Checks the current set of shells (with their associated holes) to
* see if any of them contain the point.
*/
bool PolygonBuilder::containsPoint(Coordinate& p) {
	for(int i=0;i<(int)shellList->size();i++) {
		EdgeRing *er=(*shellList)[i];
		if (er->containsPoint(p))
			return true;
	}
	return false;
}
}

