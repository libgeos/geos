#include "graph.h"
#include "util.h"

EdgeEndStar::EdgeEndStar(){
	ptInAreaLocation[0]=Location::UNDEF;
	ptInAreaLocation[1]=Location::UNDEF;
	edgeMap=new set<EdgeEnd*,EdgeEndLT>();
	edgeList=new vector<EdgeEnd*>();
}

EdgeEndStar::EdgeEndStar(const EdgeEndStar &ees){
	ptInAreaLocation[0]=Location::UNDEF;
	ptInAreaLocation[1]=Location::UNDEF;
	edgeMap=new set<EdgeEnd*,EdgeEndLT>((ees.edgeMap)->begin(),(ees.edgeMap)->end());
	edgeList=new vector<EdgeEnd*>((ees.edgeList)->begin(),(ees.edgeList)->end());
}

EdgeEndStar::~EdgeEndStar(){
	delete edgeMap;
	delete edgeList;
}

/**
 * Insert an EdgeEnd into the map, and clear the edgeList cache,
 * since the list of edges has now changed
 */
void EdgeEndStar::insertEdgeEnd(EdgeEnd *e){
	edgeMap->insert(e);
	edgeList=NULL;  // edge list has changed - clear the cache
}

/**
 * @return the coordinate for the node this star is based at
 */
Coordinate EdgeEndStar::getCoordinate(){
	vector<EdgeEnd*>::iterator it=getIterator();
	if (it==NULL) return Coordinate::getNull();
	EdgeEnd *e=*it;
	return e->getCoordinate();
}

int EdgeEndStar::getDegree(){
	return (int)edgeMap->size();
}

vector<EdgeEnd*>* EdgeEndStar::getEdges() {
	if (edgeList==NULL) {
		edgeList=new vector<EdgeEnd*>(edgeMap->begin(),edgeMap->end());
	}
	return edgeList;
}

vector<EdgeEnd*>::iterator EdgeEndStar::getIterator(){
	return getEdges()->begin();
}

EdgeEnd* EdgeEndStar::getNextCW(EdgeEnd *ee){
	getEdges();
	int i;
	for(unsigned int j=0;j<edgeList->size();j++)
		if (ee->compareTo(*(edgeList->at(j)))==0) {
			i=j;
			break;
		}
	int iNextCW=i-1;
	if (i==0)
		iNextCW=(int)edgeList->size()-1;
	return edgeList->at(iNextCW);
}

void EdgeEndStar::computeLabelling(vector<GeometryGraph*> geom){
	computeEdgeEndLabels();
	// Propagate side labels  around the edges in the star
	// for each parent Geometry
	propagateSideLabels(0);
	propagateSideLabels(1);

	/**
	* If there are edges that still have null labels for a geometry
	* this must be because there are no area edges for that geometry incident on this node.
	* In this case, to label the edge for that geometry we must test whether the
	* edge is in the interior of the geometry.
	* To do this it suffices to determine whether the node for the edge is in the interior of an area.
	* If so, the edge has location INTERIOR for the geometry.
	* In all other cases (e.g. the node is on a line, on a point, or not on the geometry at all) the edge
	* has the location EXTERIOR for the geometry.
	* <p>
	* Note that the edge cannot be on the BOUNDARY of the geometry, since then
	* there would have been a parallel edge from the Geometry at this node also labelled BOUNDARY
	* and this edge would have been labelled in the previous step.
	* <p>
	* This code causes a problem when dimensional collapses are present, since it may try and
	* determine the location of a node where a dimensional collapse has occurred.
	* The point should be considered to be on the EXTERIOR
	* of the polygon, but locate() will return INTERIOR, since it is passed
	* the original Geometry, not the collapsed version.
	*
	* If there are incident edges which are Line edges labelled BOUNDARY,
	* then they must be edges resulting from dimensional collapses.
	* In this case the other edges can be labelled EXTERIOR for this Geometry.
	*
	* MD 8/11/01 - NOT TRUE!  The collapsed edges may in fact be in the interior of the Geometry,
	* which means the other edges should be labelled INTERIOR for this Geometry.
	* Not sure how solve this...  Possibly labelling needs to be split into several phases:
	* area label propagation, symLabel merging, then finally null label resolution.
	*/
	bool hasDimensionalCollapseEdge[2]={false,false};
	for (vector<EdgeEnd*>::iterator it=getIterator();it<=edgeList->end();it++) {
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		for(int geomi=0; geomi<2; geomi++) {
			if (label->isLine(geomi) && label->getLocation(geomi)==Location::BOUNDARY)
				hasDimensionalCollapseEdge[geomi]=true;
		}
	}
	for (vector<EdgeEnd*>::iterator it=getIterator();it<=edgeList->end();it++) {
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		for(int geomi=0;geomi<2;geomi++){
			if (label->isAnyNull(geomi)) {
				int loc=Location::UNDEF;
				if (hasDimensionalCollapseEdge[geomi]){
					loc=Location::EXTERIOR;
				}else {
					Coordinate p(e->getCoordinate());
					loc=getLocation(geomi,p,geom);
				}
				label->setAllLocationsIfNull(geomi,loc);
			}
		}
	}
}

void EdgeEndStar::computeEdgeEndLabels(){
	// Compute edge label for each EdgeEnd
	for (vector<EdgeEnd*>::iterator it=getIterator();it<=edgeList->end();it++) {
		EdgeEnd *e=*it;
		e->computeLabel();
	}
}

int EdgeEndStar::getLocation(int geomIndex,Coordinate p,vector<GeometryGraph*> geom){
	// compute location only on demand
	if (ptInAreaLocation[geomIndex]==Location::UNDEF) {
//!!!External Dependency
//		ptInAreaLocation[geomIndex]=SimplePointInAreaLocator::locate(p,geom[geomIndex].getGeometry());
	}
	return ptInAreaLocation[geomIndex];
}

bool EdgeEndStar::isAreaLabelsConsistent(){
	computeEdgeEndLabels();
	return checkAreaLabelsConsistent(0);
}

bool EdgeEndStar::checkAreaLabelsConsistent(int geomIndex){
	// Since edges are stored in CCW order around the node,
	// As we move around the ring we move from the right to the left side of the edge
	vector<EdgeEnd*> *edges=getEdges();
	// if no edges, trivially consistent
	if (edges->size()<=0)
		return true;
	// initialize startLoc to location of last L side (if any)
	int lastEdgeIndex=(int)edges->size()-1;
	Label *startLabel=(edges->at(lastEdgeIndex))->getLabel();
	int startLoc=startLabel->getLocation(geomIndex,Position::LEFT);
	Assert::isTrue(startLoc!=Location::UNDEF, "Found unlabelled area edge");
	int currLoc=startLoc;
	for (vector<EdgeEnd*>::iterator it=getIterator();it<=edgeList->end();it++) {
		EdgeEnd *e=*it;
		Label *eLabel=e->getLabel();
		// we assume that we are only checking a area
		Assert::isTrue(eLabel->isArea(geomIndex), "Found non-area edge");
		int leftLoc=eLabel->getLocation(geomIndex,Position::LEFT);
		int rightLoc=eLabel->getLocation(geomIndex,Position::RIGHT);
		// check that edge is really a boundary between inside and outside!
		if (leftLoc==rightLoc) {
			return false;
		}
		// check side location conflict
		//Assert.isTrue(rightLoc == currLoc, "side location conflict " + locStr);
		if (rightLoc!=currLoc) {
			return false;
		}
		currLoc=leftLoc;
	}
	return true;
}

void EdgeEndStar::propagateSideLabels(int geomIndex){
	// Since edges are stored in CCW order around the node,
	// As we move around the ring we move from the right to the left side of the edge
	int startLoc=Location::UNDEF ;
	// initialize loc to location of last L side (if any)
	for (vector<EdgeEnd*>::iterator it=getIterator();it<=edgeList->end();it++) {
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		if (label->isArea(geomIndex) && label->getLocation(geomIndex,Position::LEFT)!=Location::UNDEF)
			startLoc=label->getLocation(geomIndex,Position::LEFT);
		}
		// no labelled sides found, so no labels to propagate
	if (startLoc==Location::UNDEF) return;
	int currLoc=startLoc;
	for (vector<EdgeEnd*>::iterator it=getIterator();it<=edgeList->end();it++) {
		EdgeEnd *e=*it;
		Label *label=e->getLabel();
		// set null ON values to be in current location
		if (label->getLocation(geomIndex,Position::ON)==Location::UNDEF)
			label->setLocation(geomIndex,Position::ON,currLoc);
		// set side labels (if any)
		// if (label.isArea()) {   //ORIGINAL
		if (label->isArea(geomIndex)) {
			int leftLoc=label->getLocation(geomIndex,Position::LEFT);
			int rightLoc=label->getLocation(geomIndex,Position::RIGHT);
			// if there is a right location, that is the next location to propagate
			if (rightLoc!=Location::UNDEF) {
				string locStr="(at " + (e->getCoordinate()).toString() + ")";
				//Debug.print(rightLoc != currLoc, this);
				Assert::isTrue(rightLoc==currLoc, "side location conflict " + locStr);
				Assert::isTrue(leftLoc!=Location::UNDEF, "found single null side " + locStr);
				currLoc=leftLoc;
			} else {
				/** RHS is null - LHS must be null too.
				 *  This must be an edge from the other geometry, which has no location
				 *  labelling for this geometry.  This edge must lie wholly inside or outside
				 *  the other geometry (which is determined by the current location).
				 *  Assign both sides to be the current location.
				 */
				Assert::isTrue(label->getLocation(geomIndex,Position::LEFT)==Location::UNDEF, "found single null side");
				label->setLocation(geomIndex,Position::RIGHT, currLoc);
				label->setLocation(geomIndex,Position::LEFT, currLoc);
			}
		}
	}
}

int EdgeEndStar::findIndex(EdgeEnd *eSearch){
	getIterator();   // force edgelist to be computed
	for (unsigned int i=0; i<edgeList->size(); i++ ) {
		EdgeEnd *e=edgeList->at(i);
		if (e->compareTo(*eSearch)) return i;
	}
	return -1;
}

string EdgeEndStar::print(){
	string out="EdgeEndStar:   " + getCoordinate().toString();
	for (vector<EdgeEnd*>::iterator it=getIterator();it<=edgeList->end();it++) {
		EdgeEnd *e=*it;
		out+=e->print();
	}
	return out;
}