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
 * Revision 1.1  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/opBuffer.h"

namespace geos {

/**
* Compute the change in depth as an edge is crossed from R to L
*/
int BufferBuilder::depthDelta(Label *label) {
	int lLoc=label->getLocation(0, Position::LEFT);
	int rLoc=label->getLocation(0, Position::RIGHT);
	if (lLoc== Location::INTERIOR && rLoc== Location::EXTERIOR)
		return 1;
	else if (lLoc== Location::EXTERIOR && rLoc== Location::INTERIOR)
		return -1;
	return 0;
}

CGAlgorithms* BufferBuilder::cga=new RobustCGAlgorithms();

/**
* Creates a new BufferBuilder
*/
BufferBuilder::BufferBuilder() {
	quadrantSegments=OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS;
	endCapStyle=BufferOp::CAP_ROUND;
	edgeList=new EdgeList();
}

BufferBuilder::~BufferBuilder() {
	delete edgeList;
}

/**
* Sets the number of segments used to approximate a angle fillet
*
* @param quadrantSegments the number of segments in a fillet for a quadrant
*/
void BufferBuilder::setQuadrantSegments(int nQuadrantSegments){
	quadrantSegments=nQuadrantSegments;
}

/**
* Sets the precision model to use during the curve computation and noding,
* if it is different to the precision model of the Geometry->
* If the precision model is less than the precision of the Geometry precision model,
* the Geometry must have previously been rounded to that precision->
*
* @param pm the precision model to use
*/
void BufferBuilder::setWorkingPrecisionModel(PrecisionModel *pm){
	workingPrecisionModel=pm;
}

void BufferBuilder::setEndCapStyle(int nEndCapStyle){
	endCapStyle=nEndCapStyle;
}

Geometry* BufferBuilder::buffer(Geometry *g, double distance){
	PrecisionModel *precisionModel=workingPrecisionModel;
	if (precisionModel==NULL)
		precisionModel=(PrecisionModel*) g->getPrecisionModel();
	// factory must be the same as the one used by the input
	geomFact=g->getFactory();
	OffsetCurveBuilder *curveBuilder=new OffsetCurveBuilder(precisionModel, quadrantSegments);
	curveBuilder->setEndCapStyle(endCapStyle);
	OffsetCurveSetBuilder *curveSetBuilder=new OffsetCurveSetBuilder(g, distance, curveBuilder);
	vector<SegmentString*> *bufferSegStrList=curveSetBuilder->getCurves();
	// short-circuit test
	if ((int)bufferSegStrList->size()<=0) {
		Geometry *emptyGeom=geomFact->createGeometryCollection(new vector<Geometry*>());
		return emptyGeom;
	}
	computeNodedEdges(bufferSegStrList, precisionModel);
	graph=new PlanarGraph(new OverlayNodeFactory());
	graph->addEdges(edgeList->getEdges());
	vector<BufferSubgraph*> *subgraphList=createSubgraphs(graph);
	PolygonBuilder *polyBuilder=new PolygonBuilder(geomFact,cga);
	buildSubgraphs(subgraphList, polyBuilder);
	vector<Geometry*> *resultPolyList=polyBuilder->getPolygons();
	Geometry* resultGeom=geomFact->buildGeometry(resultPolyList);
	return resultGeom;
}

void BufferBuilder::computeNodedEdges(vector<SegmentString*> *bufferSegStrList, PrecisionModel *precisionModel){
	//BufferCurveGraphNoder noder=new BufferCurveGraphNoder(geomFact->getPrecisionModel());
	IteratedNoder *noder=new IteratedNoder(precisionModel);
	vector<SegmentString*> *nodedSegStrings=noder->node(bufferSegStrList);
	// DEBUGGING ONLY
	//BufferDebug->saveEdges(nodedEdges, "run" + BufferDebug->runCount + "_nodedEdges");
	for (int i=0;i<(int)nodedSegStrings->size();i++) {
		SegmentString *segStr=(*nodedSegStrings)[i];
		Label *oldLabel=(Label*) segStr->getContext();
		Edge *edge=new Edge((CoordinateList*) segStr->getCoordinates(), new Label(oldLabel));
		insertEdge(edge);
	}
	//saveEdges(edgeList->getEdges(), "run" + runCount + "_collapsedEdges");
}


/**
* Inserted edges are checked to see if an identical edge already exists->
* If so, the edge is not inserted, but its label is merged
* with the existing edge->
*/
void BufferBuilder::insertEdge(Edge *e){
	//<FIX> MD 8 Oct 03  speed up identical edge lookup
	// fast lookup
	Edge *existingEdge=edgeList->findEqualEdge(e);
	// If an identical edge already exists, simply update its label
	if (existingEdge != NULL) {
		Label *existingLabel=existingEdge->getLabel();
		Label *labelToMerge=e->getLabel();
		// check if new edge is in reverse direction to existing edge
		// if so, must flip the label before merging it
		if (! existingEdge->isPointwiseEqual(e)) {
			labelToMerge=new Label(e->getLabel());
			labelToMerge->flip();
		}
		existingLabel->merge(labelToMerge);
		// compute new depth delta of sum of edges
		int mergeDelta=depthDelta(labelToMerge);
		int existingDelta=existingEdge->getDepthDelta();
		int newDelta=existingDelta + mergeDelta;
		existingEdge->setDepthDelta(newDelta);
	} else {   // no matching existing edge was found
		// add this new edge to the list of edges in this graph
		//e->setName(name + edges->size());
		edgeList->add(e);
		e->setDepthDelta(depthDelta(e->getLabel()));
	}
}

bool BufferSubgraphGT(BufferSubgraph *first, BufferSubgraph *second) {
	if (first->compareTo(second)>=0)
		return true;
	else
		return false;
}

vector<BufferSubgraph*>* BufferBuilder::createSubgraphs(PlanarGraph *graph){
	vector<BufferSubgraph*> *subgraphList=new vector<BufferSubgraph*>();
	vector<Node*> *n=graph->getNodes();
	for (int i=0;i<(int)n->size();i++) {
		Node *node=(*n)[i];
		if (!node->isVisited()) {
			BufferSubgraph *subgraph=new BufferSubgraph(cga);
			subgraph->create(node);
			subgraphList->push_back(subgraph);
		}
	}
	/**
	* Sort the subgraphs in descending order of their rightmost coordinate->
	* This ensures that when the Polygons for the subgraphs are built,
	* subgraphs for shells will have been built before the subgraphs for
	* any holes they contain->
	*/
	sort(subgraphList->begin(),subgraphList->end(),BufferSubgraphGT);
	return subgraphList;
}

/**
* Completes the building of the input subgraphs by depth-labelling them,
* and adds them to the PolygonBuilder->
* The subgraph list must be sorted in rightmost-coordinate order->
*
* @param subgraphList the subgraphs to build
* @param polyBuilder the PolygonBuilder which will build the final polygons
*/
void BufferBuilder::buildSubgraphs(vector<BufferSubgraph*> *subgraphList,PolygonBuilder *polyBuilder){
	vector<BufferSubgraph*> *processedGraphs=new vector<BufferSubgraph*>();
	for (int i=0;i<(int)subgraphList->size();i++) {
		BufferSubgraph *subgraph=(*subgraphList)[i];
		Coordinate *p=subgraph->getRightmostCoordinate();
		SubgraphDepthLocater *locater=new SubgraphDepthLocater(processedGraphs);
		int outsideDepth=locater->getDepth(*p);
		subgraph->computeDepth(outsideDepth);
		subgraph->findResultEdges();
		processedGraphs->push_back(subgraph);
		polyBuilder->add(subgraph->getDirectedEdges(), subgraph->getNodes());
	}
}
}
