#include "graphindex.h"
#include "math.h"

bool SegmentIntersector::isAdjacentSegments(int i1,int i2){
	return abs(i1-i2)==1;
}

SegmentIntersector::SegmentIntersector(){
	hasIntersectionVar=false;
	hasProper=false;
	hasProperInterior=false;
	numIntersections=0;
	numTests=0;
}

SegmentIntersector::SegmentIntersector(LineIntersector *newLi,bool newIncludeProper,bool newRecordIsolated){
	hasIntersectionVar=false;
	hasProper=false;
	hasProperInterior=false;
	numIntersections=0;
	numTests=0;

	li=newLi;
	includeProper=newIncludeProper;
	recordIsolated=newRecordIsolated;
}

void SegmentIntersector::setBoundaryNodes(vector<Node*> *bdyNodes0,vector<Node*> *bdyNodes1){
	bdyNodes.resize(2);
	bdyNodes[0]=bdyNodes0;
	bdyNodes[1]=bdyNodes1;
}

/**
* @return the proper intersection point, or <code>null</code> if none was found
*/
Coordinate SegmentIntersector::getProperIntersectionPoint(){
	return properIntersectionPoint;
}

bool SegmentIntersector::hasIntersection() {
	return hasIntersectionVar;
}

/**
* A proper intersection is an intersection which is interior to at least two
* line segments.  Note that a proper intersection is not necessarily
* in the interior of the entire Geometry, since another edge may have
* an endpoint equal to the intersection, which according to SFS semantics
* can result in the point being on the Boundary of the Geometry.
*/
bool SegmentIntersector::hasProperIntersection() {
	return hasProper;
}

/**
 * A proper interior intersection is a proper intersection which is <b>not</b>
 * contained in the set of boundary nodes set for this SegmentIntersector.
 */
bool SegmentIntersector::hasProperInteriorIntersection() {
	return hasProperInterior;
}

/**
* A trivial intersection is an apparent self-intersection which in fact
* is simply the point shared by adjacent line segments.
* Note that closed edges require a special check for the point shared by the beginning
* and end segments.
*/
bool SegmentIntersector::isTrivialIntersection(Edge *e0,int segIndex0,Edge *e1,int segIndex1){
	if (*e0==*e1) {
		if (li->getIntersectionNum()==1) {
			if (isAdjacentSegments(segIndex0,segIndex1))
				return true;
			if (e0->isClosed()) {
				int maxSegIndex=e0->getNumPoints()-1;
				if ((segIndex0==0 && segIndex1==maxSegIndex)
					|| (segIndex1==0 && segIndex0==maxSegIndex)) {
					return true;
				}
			}
		}
	}
	return false;
}

/**
* This method is called by clients of the EdgeIntersector class to test for and add
* intersections for two segments of the edges being intersected.
* Note that clients (such as MonotoneChainEdges) may choose not to intersect
* certain pairs of segments for efficiency reasons.
*/
void SegmentIntersector::addIntersections(Edge *e0,int segIndex0,Edge *e1,int segIndex1){
	if (*e0==*e1 && segIndex0==segIndex1) return;
	numTests++;
	Coordinate p00(e0->getCoordinates().getAt(segIndex0));
	Coordinate p01(e0->getCoordinates().getAt(segIndex0+1));
	Coordinate p10(e1->getCoordinates().getAt(segIndex1));
	Coordinate p11(e1->getCoordinates().getAt(segIndex1+1));
	li->computeIntersection(p00,p01,p10,p11);
	/**
	*  Always record any non-proper intersections.
	*  If includeProper is true, record any proper intersections as well.
	*/
	if (li->hasIntersection()) {
		if (recordIsolated) {
			e0->setIsolated(false);
			e1->setIsolated(false);
		}
		//intersectionFound = true;
		numIntersections++;
		// if the segments are adjacent they have at least one trivial intersection,
		// the shared endpoint.  Don't bother adding it if it is the
		// only intersection.
		if (!isTrivialIntersection(e0,segIndex0,e1,segIndex1)) {
			hasIntersectionVar=true;
			if (includeProper || !li->isProper()) {
				//Debug.println(li);
				e0->addIntersections(li,segIndex0,0);
				e1->addIntersections(li,segIndex1,1);
			}
			if (li->isProper()) {
				properIntersectionPoint.setCoordinate(li->getIntersection(0));
				hasProper=true;
				if (!isBoundaryPoint(li,&bdyNodes))
					hasProperInterior=true;
			}
			//if (li.isCollinear())
			//hasCollinear = true;
		}
	}
}

bool SegmentIntersector::isBoundaryPoint(LineIntersector *li,vector<vector<Node*>*> *tstBdyNodes) {
	if (tstBdyNodes==NULL) return false;
	if (isBoundaryPoint(li,tstBdyNodes->at(0))) return true;
	if (isBoundaryPoint(li,tstBdyNodes->at(1))) return true;
	return false;
}

bool SegmentIntersector::isBoundaryPoint(LineIntersector *li,vector<Node*> *tstBdyNodes){
	for(vector<Node*>::iterator i=tstBdyNodes->begin();i<=tstBdyNodes->end();i++) {
		Node *node=*i;
		Coordinate pt(node->getCoordinate());
		if (li->isIntersection(pt)) return true;
	}
	return false;
}
