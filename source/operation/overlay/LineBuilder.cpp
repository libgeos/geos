#include "../../headers/opOverlay.h"
#include "stdio.h"

namespace geos {

LineBuilder::LineBuilder(OverlayOp *newOp,GeometryFactory *newGeometryFactory,PointLocator *newPtLocator) {
	op=newOp;
	geometryFactory=newGeometryFactory;
	ptLocator=newPtLocator;
	lineEdgesList=new vector<Edge*>();
	resultLineList=new vector<LineString*>();
}

LineBuilder::~LineBuilder() {
	delete lineEdgesList;
}

/**
* @return a list of the LineStrings in the result of the specified overlay operation
*/
vector<LineString*>* LineBuilder::build(int opCode){
	findCoveredLineEdges();
	collectLines(opCode);
	//labelIsolatedLines(lineEdgesList);
	buildLines(opCode);
	return resultLineList;
}

/**
* Find and mark L edges which are "covered" by the result area (if any).
* L edges at nodes which also have A edges can be checked by checking
* their depth at that node.
* L edges at nodes which do not have A edges can be checked by doing a
* point-in-polygon test with the previously computed result areas.
*/
void LineBuilder::findCoveredLineEdges() {
// first set covered for all L edges at nodes which have A edges too
	map<Coordinate,Node*,CoordLT> *nodeMap=op->getGraph()->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		//node.print(System.out);
		((DirectedEdgeStar*)node->getEdges())->findCoveredLineEdges();
	}

	/**
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

void LineBuilder::collectLines(int opCode) {
	vector<EdgeEnd*> *ee=op->getGraph()->getEdgeEnds();
	for(int i=0;i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*) (*ee)[i];
		collectLineEdge(de,opCode,lineEdgesList);
		collectBoundaryTouchEdge(de,opCode,lineEdgesList);
	}
}

void LineBuilder::collectLineEdge(DirectedEdge *de,int opCode,vector<Edge*> *edges) {
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

/**
* Collect edges from Area inputs which should be in the result but
* which have not been included in a result area.
* This happens ONLY:
* <ul>
* <li>during an intersection when the boundaries of two
* areas touch in a line segment
* <li> OR as a result of a dimensional collapse.
* </ul>
*/
void LineBuilder::collectBoundaryTouchEdge(DirectedEdge *de,int opCode,vector<Edge*> *edges){
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

void LineBuilder::buildLines(int opCode) {
	// need to simplify lines?
	for(int i=0;i<(int)lineEdgesList->size();i++) {
		Edge *e=(*lineEdgesList)[i];
		Label *label=e->getLabel();
		//e.print(System.out);
		////System.out.println(label);
		//if (OverlayGraph.isResultOfOp(label, opCode)) {
		LineString *line=geometryFactory->createLineString(CoordinateListFactory::internalFactory->createCoordinateList(e->getCoordinates()));
			resultLineList->push_back(line);
			e->setInResult(true);
		//}
	}
}

void LineBuilder::labelIsolatedLines(vector<Edge*> *edgesList){
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

/**
* Label an isolated node with its relationship to the target geometry.
*/
void LineBuilder::labelIsolatedLine(Edge *e,int targetIndex) {
	int loc=ptLocator->locate(e->getCoordinate(),op->getArgGeometry(targetIndex));
	e->getLabel()->setLocation(targetIndex,loc);
}
}