#include "../headers/graph.h"

namespace geos {

/**
 * Updates an IM from the label for an edge.
 * Handles edges from both L and A geometrys.
 */
void Edge::updateIM(Label *lbl,IntersectionMatrix *im){
	im->setAtLeastIfValid(lbl->getLocation(0,Position::ON),lbl->getLocation(1,Position::ON),1);
	if (lbl->isArea()) {
		im->setAtLeastIfValid(lbl->getLocation(0,Position::LEFT),lbl->getLocation(1,Position::LEFT),2);
		im->setAtLeastIfValid(lbl->getLocation(0,Position::RIGHT),lbl->getLocation(1,Position::RIGHT),2);
	}
}

Edge::Edge(){
	eiList=NULL;
	isIsolatedVar=true;
	depth=NULL;
	depthDelta=0;
	mce=NULL;
	pts=NULL;
}

Edge::~Edge(){
	delete eiList;
	delete depth;
	delete mce;
	delete pts;
}

Edge::Edge(CoordinateList* newPts, Label *newLabel):GraphComponent(newLabel){
	eiList=new EdgeIntersectionList(this);
	isIsolatedVar=true;
	depth=new Depth();
	depthDelta=0;
//	label=newLabel;
	mce=NULL;
	pts=newPts;
}

Edge::Edge(CoordinateList* newPts){
	eiList=new EdgeIntersectionList(this);
	isIsolatedVar=true;
	depth=new Depth();
	depthDelta=0;
	label=NULL;
	mce=NULL;
	pts=newPts;
}

int Edge::getNumPoints() {
	return pts->getSize();
}

void Edge::setName(string newName) {
	name=newName;
}

//CoordinateList Edge::getCoordinates(){
//	return pts;
//}

CoordinateList* Edge::getCoordinates(){
	return pts;
}

Coordinate& Edge::getCoordinate(int i){
	return pts->getAt(i);
}

Coordinate& Edge::getCoordinate(){
	if (pts->getSize()>0) return pts->getAt(0);
	return *(new Coordinate(DoubleNotANumber,DoubleNotANumber,DoubleNotANumber));
}

Depth* Edge::getDepth() {
	return depth;
}

int Edge::getDepthDelta(){
	return depthDelta;
}

void Edge::setDepthDelta(int newDepthDelta){
	depthDelta=newDepthDelta;
}

int Edge::getMaximumSegmentIndex(){
	return pts->getSize()-1;
}

EdgeIntersectionList* Edge::getEdgeIntersectionList() {
	return eiList;
}

MonotoneChainEdge* Edge::getMonotoneChainEdge(){
	if (mce==NULL) mce=new MonotoneChainEdge(this);
	return mce;
}

bool Edge::isClosed(){
	return pts->getAt(0)==pts->getAt(pts->getSize()-1);
}

/**
 * An Edge is collapsed if it is an Area edge and it consists of
 * two segments which are equal and opposite (eg a zero-width V).
 */
bool Edge::isCollapsed(){
	if (!label->isArea()) return false;
	if (pts->getSize()!= 3) return false;
	if (pts->getAt(0)==pts->getAt(2) ) return true;
	return false;
}

Edge* Edge::getCollapsedEdge() {
	CoordinateList *newPts=CoordinateListFactory::internalFactory->createCoordinateList(2);
	newPts->setAt(pts->getAt(0),0);
	newPts->setAt(pts->getAt(1),1);
	return new Edge(newPts,Label::toLineLabel(label));
}

void Edge::setIsolated(bool newIsIsolated){
	isIsolatedVar=newIsIsolated;
}

bool Edge::isIsolated(){
	return isIsolatedVar;
}

/**
 * Adds EdgeIntersections for one or both
 * intersections found for a segment of an edge to the edge intersection list.
 */
void Edge::addIntersections(LineIntersector *li, int segmentIndex, int geomIndex){
	for (int i=0; i<li->getIntersectionNum();i++) {
		addIntersection(li,segmentIndex,geomIndex,i);
	}
}

/**
 * Add an EdgeIntersection for intersection intIndex.
 * An intersection that falls exactly on a vertex of the edge is normalized
 * to use the higher of the two possible segmentIndexes
 */
void Edge::addIntersection(LineIntersector *li,int segmentIndex,int geomIndex,int intIndex){
	Coordinate& intPt=li->getIntersection(intIndex);
	int normalizedSegmentIndex=segmentIndex;
	double dist=li->getEdgeDistance(geomIndex,intIndex);
	// normalize the intersection point location
	int nextSegIndex=normalizedSegmentIndex+1;
	if (nextSegIndex<pts->getSize()) {
		Coordinate& nextPt=pts->getAt(nextSegIndex);
        // Normalize segment index if intPt falls on vertex
        // The check for point equality is 2D only - Z values are ignored
		if (intPt.equals2D(nextPt)) {
			normalizedSegmentIndex=nextSegIndex;
			dist=0.0;
		}
	}
	/**
	 * Add the intersection point to edge intersection list.
	 */
	EdgeIntersection *ei=eiList->add(intPt,normalizedSegmentIndex,dist);
}

/**
 * Update the IM with the contribution for this component.
 * A component only contributes if it has a labelling for both parent geometries
 */
void Edge::computeIM(IntersectionMatrix *im){
	updateIM(label,im);
}

/**
 * equals is defined to be:
 * <p>
 * e1 equals e2
 * <b>iff</b>
 * the coordinates of e1 are the same or the reverse of the coordinates in e2
 */
bool operator==(Edge e1, Edge e2){
	if (e1.pts->getSize()!=e2.pts->getSize()) return false;
	bool isEqualForward=true;
	bool isEqualReverse=true;
	int iRev=e1.pts->getSize();
	for (int i=0; i<e1.pts->getSize();i++) {
		if (!e1.pts->getAt(i).equals2D(e2.pts->getAt(i))) {
			isEqualForward=false;
		}
		if (!e1.pts->getAt(i).equals2D(e2.pts->getAt(--iRev))) {
			isEqualReverse=false;
		}
		if (!isEqualForward && !isEqualReverse) return false;
	}
	return true;
}

/**
 * equals is defined to be:
 * <p>
 * e1 equals e2
 * <b>iff</b>
 * the coordinates of e1 are the same or the reverse of the coordinates in e2
 */
bool Edge::equals(Edge *e){
	if (pts->getSize()!=e->pts->getSize()) return false;
	bool isEqualForward=true;
	bool isEqualReverse=true;
	int iRev=pts->getSize();
	for (int i=0; i<pts->getSize();i++) {
		if (!pts->getAt(i).equals2D(e->pts->getAt(i))) {
			isEqualForward=false;
		}
		if (!pts->getAt(i).equals2D(e->pts->getAt(--iRev))) {
			isEqualReverse=false;
		}
		if (!isEqualForward && !isEqualReverse) return false;
	}
	return true;
}

/**
 * @return true if the coordinate sequences of the Edges are identical
 */
bool Edge::isPointwiseEqual(Edge *e){
	if (pts->getSize()!=e->pts->getSize()) return false;
	for (int i=0;i<pts->getSize();i++) {
		if (!pts->getAt(i).equals2D(e->pts->getAt(i))) {
			return false;
		}
	}
	return true;
}

string Edge::print(){
	string out="edge " + name + ": ";
	out+="LINESTRING (";
	for(int i=0; i<pts->getSize();i++) {
		if (i>0) out+=",";
		out+=pts->getAt(i).toString();
	}
	out+=")  ";
	out+=label->toString();
	out+=" ";
	out+=depthDelta;
	return out;
}
  
string Edge::printReverse(){
	string out="edge " + name + ": ";
	for(int i=pts->getSize()-1;i>=0;i--) {
		out+=pts->getAt(i).toString() + " ";
	}
	out+="\n";
	return out;
}
}