#include "graph.h"
#include "math.h"

/**
 * Computes the factor for the change in depth when moving from one location to another.
 * E.g. if crossing from the INTERIOR to the EXTERIOR the depth decreases, so the factor is -1
 */
int DirectedEdge::depthFactor(int currLocation, int nextLocation){
	if (currLocation==Location::EXTERIOR && nextLocation==Location::INTERIOR)
		return 1;
	else if (currLocation==Location::INTERIOR && nextLocation==Location::EXTERIOR)
		return -1;
	return 0;
}

DirectedEdge::DirectedEdge() {
	isInResultVar=false;
	isVisitedVar=false;
	depth[0]=0;
	depth[1]=-999;
	depth[2]=-999;
}

DirectedEdge::DirectedEdge(Edge *newEdge, bool newIsForward): EdgeEnd(newEdge){
	isInResultVar=false;
	isVisitedVar=false;
	depth[0]=0;
	depth[1]=-999;
	depth[2]=-999;

	isForwardVar=newIsForward;
	if (isForwardVar) {
		init(edge->getCoordinate(0), edge->getCoordinate(1));
	} else {
		int n=edge->getNumPoints()-1;
		init(edge->getCoordinate(n), edge->getCoordinate(n-1));
	}
	computeDirectedLabel();
}

Edge* DirectedEdge::getEdge() {
	return edge;
}

void DirectedEdge::setInResult(bool newIsInResult) {
	isInResultVar=newIsInResult;
}

bool DirectedEdge::isInResult() {
	return isInResultVar;
}

bool DirectedEdge::isVisited() {
	return isVisitedVar;
}

void DirectedEdge::setVisited(bool newIsVisited) {
	isVisitedVar=newIsVisited;
}

void DirectedEdge::setEdgeRing(EdgeRing *newEdgeRing) {
	edgeRing=newEdgeRing;
}

EdgeRing* DirectedEdge::getEdgeRing() {
	return edgeRing;
}

void DirectedEdge::setMinEdgeRing(EdgeRing *newMinEdgeRing) {
	minEdgeRing=newMinEdgeRing;
}

EdgeRing* DirectedEdge::getMinEdgeRing() {
	return minEdgeRing;
}

int DirectedEdge::getDepth(int position){
	return depth[position];
}

void DirectedEdge::setDepth(int position, int newDepth) {
	depth[position]=newDepth;
}

/**
 * setVisitedEdge marks both DirectedEdges attached to a given Edge.
 * This is used for edges corresponding to lines, which will only
 * appear oriented in a single direction in the result.
 */
void DirectedEdge::setVisitedEdge(bool newIsVisited){
	setVisited(newIsVisited);
	sym->setVisited(newIsVisited);
}

/**
 * Each Edge gives rise to a pair of symmetric DirectedEdges, in opposite
 * directions.
 * @return the DirectedEdge for the same Edge but in the opposite direction
 */
DirectedEdge* DirectedEdge::getSym() {
	return sym;
}

bool DirectedEdge::isForward() {
	return isForwardVar;
}

void DirectedEdge::setSym(DirectedEdge *de){
	sym=de;
}

DirectedEdge* DirectedEdge::getNext() {
	return next;
}

void DirectedEdge::setNext(DirectedEdge *newNext) {
	next=newNext;
}

DirectedEdge* DirectedEdge::getNextMin() {
	return nextMin;
}

void DirectedEdge::setNextMin(DirectedEdge *newNextMin) {
	nextMin=newNextMin;
}

/**
 * This edge is a line edge if
 * <ul>
 * <li> at least one of the labels is a line label
 * <li> any labels which are not line labels have all Locations = EXTERIOR
 * </ul>
 */
bool DirectedEdge::isLineEdge() {
	bool isLine=label->isLine(0) || label->isLine(1);
	bool isExteriorIfArea0=!label->isArea(0) || label->allPositionsEqual(0,Location::EXTERIOR);
	bool isExteriorIfArea1=!label->isArea(1) || label->allPositionsEqual(1,Location::EXTERIOR);
	return isLine && isExteriorIfArea0 && isExteriorIfArea1;
}

/**
 * This is an interior Area edge if
 * <ul>
 * <li> its label is an Area label for both Geometries
 * <li> and for each Geometry both sides are in the interior.
 * </ul>
 *
 * @return true if this is an interior Area edge
 */
bool DirectedEdge::isInteriorAreaEdge(){
	bool isInteriorAreaEdge=true;
	for (int i=0; i<2; i++) {
		if (!(label->isArea(i)
			&& label->getLocation(i,Position::LEFT )==Location::INTERIOR
			&& label->getLocation(i,Position::RIGHT)==Location::INTERIOR)) {
				isInteriorAreaEdge=false;
		}
	}
	return isInteriorAreaEdge;
}

/**
 * Compute the label in the appropriate orientation for this DirEdge
 */
void DirectedEdge::computeDirectedLabel(){
	label=new Label(*(edge->getLabel()));
	if (!isForwardVar)
		label->flip();
}

/**
 * Set both edge depths.  One depth for a given side is provided.  The other is
 * computed depending on the Location transition and the depthDelta of the edge.
 */
void DirectedEdge::setEdgeDepths(int position, int newDepth){
	int depthDelta=getEdge()->getDepthDelta();
	int loc=label->getLocation(0, position);
	int oppositePos=Position::opposite(position);
	int oppositeLoc=label->getLocation(0, oppositePos);
	int delta=abs(depthDelta)*DirectedEdge::depthFactor(loc,oppositeLoc);
	int oppositeDepth=newDepth + delta;
	setDepth(position, newDepth);
	setDepth(oppositePos,oppositeDepth);
}

string DirectedEdge::print(){
	string out=EdgeEnd::print();
	out+=" ";
	out+=depth[Position::LEFT];
	out+="/";
	out+=depth[Position::RIGHT];
	if (isInResultVar) out+=" inResult";
	return out;
}

string DirectedEdge::printEdge(){
	string out=print();
	out+=" ";
	if (isForwardVar)
		out+=edge->print();
	else
		out+=edge->printReverse();
	return out;
}

