#include "../../headers/opBuffer.h"
#include "math.h"
#define PI 3.14159265358979

namespace geos {

/**
*The minimum segment length allowed to occur in the buffer line.  This is
*required to avoid problems with dimensional collapse when the buffer line
*is rounded to the precision grid.  The value to use has been determined empirically.
*It is independent of the buffer distance and the PrecisionModel scale factor.
*It does affect how true the resulting buffer is-geometries with line
*segments that are similar in size to the minimum segment length will
*produce "choppy" buffers
*/
double BufferLineBuilder::minSegmentLength=10.0;
bool BufferLineBuilder::useMinSegmentLength=false;

/**
*Computes a facet angle that is no greater than angleInc, but divides the totalAngle
*into equal slices.
*/
double BufferLineBuilder::facetAngle(double angleInc, double totalAngle){
	int nSlices=(int)(totalAngle/angleInc)+1;
	return totalAngle/nSlices;
}

/**
*Computes the angle between two vectors (p-pa) and (p-pb) using the relation:
*<p>
*a.b=|a| |b| cos theta, where a.b=ax.bx+ay.by
*
*/
double BufferLineBuilder::angleBetween(Coordinate &pa, Coordinate &p, Coordinate &pb) {
	double aDx=pa.x-p.x;
	double aDy=pa.y-p.y;
	double bDx=pb.x-p.x;
	double bDy=pb.y-p.y;
	double aDotB=aDx*bDx+aDy*bDy;
	double aSize=sqrt(aDx*aDx+aDy*aDy);
	double bSize=sqrt(bDx*bDx+bDy*bDy);
	double cosTheta=aDotB/aSize/bSize;
	double theta=acos(cosTheta);
	return theta;
}

BufferLineBuilder::BufferLineBuilder(CGAlgorithms *newCga,LineIntersector *newLi,PrecisionModel *newPM){
	BufferLineBuilder(newCga,newLi,newPM,DEFAULT_QUADRANT_SEGMENTS);
}

BufferLineBuilder::BufferLineBuilder(CGAlgorithms *newCga,LineIntersector *newLi,PrecisionModel *newPM,int quadrantSegments){
	side=0;
	distance=0.0;
	seg0=new LineSegment();
	seg1=new LineSegment();
	offset0=new LineSegment();
	offset1=new LineSegment();
	loopFilter=new LoopFilter();
	ptList=CoordinateListFactory::internalFactory->createCoordinateList();

	cga=newCga;
	li=newLi;
	precisionModel=newPM;
	int limitedQuadSegs=quadrantSegments<1 ? 1 : quadrantSegments;
	angleInc=PI/2.0/limitedQuadSegs;
	lineList=new vector<CoordinateList*>(1);
}

BufferLineBuilder::~BufferLineBuilder(){
	delete seg0;
	delete seg1;
	delete offset0;
	delete offset1;
	delete loopFilter;
	delete ptList;
	delete lineList;
}

/**
*This method handles single points as well as lines.
*Lines are assumed to NOT be closed (the function will not
*fail for closed lines, but will generate superfluous line caps).
*/
vector<CoordinateList*>* BufferLineBuilder::getLineBuffer(CoordinateList *inputPts, double distance){
	init(distance);
	if (inputPts->getSize()<=1) {
		addCircle(inputPts->getAt(0), distance);
	} else {
        computeLineBuffer(inputPts);
	}
	CoordinateList *lineCoord=getCoordinates();
	(*lineList)[0]=lineCoord;
	return lineList;
}

/**
*This method handles the degenerate cases of single points and lines,
*as well as rings.
*/
vector<CoordinateList*>* BufferLineBuilder::getRingBuffer(CoordinateList *inputPts, int side, double distance){
	init(distance);
	if (inputPts->getSize()<=1) {
		addCircle(inputPts->getAt(0),distance);
	} else if (inputPts->getSize()==2) {
		computeLineBuffer(inputPts);
	} else {
		computeRingBuffer(inputPts, side);
	}
	(*lineList)[0]=getCoordinates();
	return lineList;
}

void BufferLineBuilder::init(double newDistance) {
	distance=newDistance;
	ptList=CoordinateListFactory::internalFactory->createCoordinateList();
}

CoordinateList* BufferLineBuilder::getCoordinates(){
	// check that points are a ring-add the startpoint again if they are not
	if (ptList->getSize()>1) {
		Coordinate &start=ptList->getAt(0);
		Coordinate &end=ptList->getAt(1);
		if (!(start==end)) addPt(start);
	}
	return ptList;
	//Coordinate[] filterCoord=loopFilter.filter(coord);
	//return filterCoord;
}

void BufferLineBuilder::computeLineBuffer(CoordinateList *inputPts){
	int n=inputPts->getSize()-1;
	// compute points for left side of line
	initSideSegments(inputPts->getAt(0),inputPts->getAt(1),Position::LEFT);
	for (int i=2;i<=n;i++) {
		addNextSegment(inputPts->getAt(i),true);
	}
	addLastSegment();
	addLineEndCap(inputPts->getAt(n-1),inputPts->getAt(n));
	// compute points for right side of line
	initSideSegments(inputPts->getAt(n),inputPts->getAt(n-1),Position::LEFT);
	for (int i=n-2;i>=0;i--) {
		addNextSegment(inputPts->getAt(i),true);
	}
	addLastSegment();
	addLineEndCap(inputPts->getAt(1),inputPts->getAt(0));
	closePts();
}

void BufferLineBuilder::computeRingBuffer(CoordinateList *inputPts, int side){
	int n=inputPts->getSize()-1;
	initSideSegments(inputPts->getAt(n-1),inputPts->getAt(0), side);
	for (int i=1;i<=n;i++) {
	bool addStartPoint=i!=1;
		addNextSegment(inputPts->getAt(i), addStartPoint);
	}
	closePts();
}

void BufferLineBuilder::addPt(Coordinate &pt) {
	Coordinate& bufPt=*(new Coordinate(pt));
	precisionModel->makePrecise(&bufPt);
	// don't add duplicate points
	Coordinate lastPt;
	if (ptList->getSize()>=1)
		lastPt=ptList->getAt(ptList->getSize()-1);
	if (!(lastPt==Coordinate::getNull()) && bufPt==lastPt) return;
	// if new segment is shorter than tolerance length, skip it
	if (useMinSegmentLength) {
		if (!(lastPt==Coordinate::getNull()) && bufPt.distance(lastPt)<minSegmentLength) return;
	}
	ptList->add(bufPt);
	//System.out.println(bufPt);
}

void BufferLineBuilder::closePts(){
	if (ptList->getSize()<1) return;
	Coordinate& startPt=*(new Coordinate(ptList->getAt(0)));
	Coordinate& lastPt=ptList->getAt(ptList->getSize()-1);
	Coordinate last2Pt;
	if (ptList->getSize()>=2)
		last2Pt=ptList->getAt(ptList->getSize()-2);
	/**
	*If the last point is too close to the start point,
	*check point n and point n-1 to see which is further from startPoint, and use
	*whichever is further as the last point
	*/
	if (useMinSegmentLength) {
		if (startPt.distance(lastPt)<minSegmentLength && !(lastPt==Coordinate::getNull())) {
			if (startPt.distance(lastPt)<startPt.distance(last2Pt)) {
				ptList->deleteAt(ptList->getSize()-1);
			}
		}
	}
	if (startPt==lastPt) return;
	ptList->add(startPt);
}

void BufferLineBuilder::initSideSegments(Coordinate &ns1, Coordinate &ns2, int nside){
	s1=ns1;
	s2=ns2;
	side=nside;
	seg1->setCoordinates(s1, s2);
	computeOffsetSegment(seg1, side, distance, offset1);
}

void BufferLineBuilder::addNextSegment(Coordinate &p, bool addStartPoint){
	s0=s1;
	s1=s2;
	s2=p;
	seg0->setCoordinates(s0, s1);
	computeOffsetSegment(seg0, side, distance, offset0);
	seg1->setCoordinates(s1, s2);
	computeOffsetSegment(seg1, side, distance, offset1);
	// do nothing if points are equal
	if (s1==s2) return;
	int orientation=cga->computeOrientation(s0, s1, s2);
	bool outsideTurn= (orientation==CGAlgorithms::CLOCKWISE && side==Position::LEFT)
					||(orientation==CGAlgorithms::COUNTERCLOCKWISE && side==Position::RIGHT);
	if (orientation==0) { // lines are collinear
		li->computeIntersection( s0, s1,s1,s2);
		int numInt=li->getIntersectionNum();
		/**
		*if numInt is<2, the lines are parallel and in the same direction.
		*In this case the point can be ignored, since the offset lines will also be
		*parallel.
		*/
		if (numInt>=2) {
			/**
			*segments are collinear but reversing.  Have to add an "end-cap" fillet
			*all the way around to other direction
			*This case should ONLY happen for LineStrings, so the orientation is always CW.
			*(Polygons can never have two consecutive segments which are parallel but reversed,
			*because that would be a self intersection.
			*/
			addFillet(s1,offset0->p1,offset1->p0,CGAlgorithms::CLOCKWISE, distance);
		}
	} else if (outsideTurn) {
		// add a fillet to connect the endpoints of the offset segments
		if (addStartPoint) addPt(offset0->p1);
		addFillet(s1, offset0->p1, offset1->p0, orientation, distance);
		addPt(offset1->p0);
	} else { // inside turn
		/**
		*add intersection point of offset segments (if any)
		*/
		li->computeIntersection(offset0->p0,offset0->p1,offset1->p0,offset1->p1);
		if (li->hasIntersection()) {
			addPt(li->getIntersection(0));
		}
			// TESTING-fix problem with narrow angles
		else {
			/**
			*If no intersection, it means the angle is so small and the offset so large
			*that the offsets segments don't intersect.  The offset segment won't appear in
			*the final buffer.  However, we can't just drop the segment, since this might
			*mean the buffer line wouldn't track the buffer correctly around the corner.
			*/
			// add both endpoint of this segment and startpoint of next
			addPt(offset0->p1);
			addPt(offset1->p0);
		}
	}
}

/**
*Add last offset point
*/
void BufferLineBuilder::addLastSegment(){
	addPt(offset1->p1);
}

void BufferLineBuilder::computeOffsetSegment(LineSegment *seg, int side, double distance, LineSegment *offset){
	int sideSign=side==Position::LEFT ? 1 : -1;
	double dx=seg->p1.x-seg->p0.x;
	double dy=seg->p1.y-seg->p0.y;
	double len=sqrt(dx*dx+dy*dy);
	// u is the vector that is the length of the offset, in the direction of the segment
	double ux=sideSign*distance*dx/len;
	double uy=sideSign*distance*dy/len;
	offset->p0.x=seg->p0.x-uy;
	offset->p0.y=seg->p0.y+ux;
	offset->p1.x=seg->p1.x-uy;
	offset->p1.y=seg->p1.y+ux;
}

/**
*Add an end cap around point p1, terminating a line segment coming from p0
*/
void BufferLineBuilder::addLineEndCap(Coordinate &p0, Coordinate &p1){
	LineSegment *seg=new LineSegment(p0,p1);
	LineSegment *offsetL=new LineSegment();
	computeOffsetSegment(seg,Position::LEFT, distance, offsetL);
	LineSegment *offsetR=new LineSegment();
	computeOffsetSegment(seg,Position::RIGHT, distance, offsetR);
	double dx=p1.x-p0.x;
	double dy=p1.y-p0.y;
	double angle=atan2(dy, dx);
	addPt(offsetL->p1);
	addFillet(p1,angle+PI/2,angle-PI/2,CGAlgorithms::CLOCKWISE, distance);
	addPt(offsetR->p1);
}
/**
*@param p is base point of curve
*@param p0 is start point of fillet curve
*@param p1 is endpoint of fillet curve
*/
void BufferLineBuilder::addFillet(Coordinate &p, Coordinate &p0, Coordinate &p1, int direction, double distance){
	double dx0=p0.x-p.x;
	double dy0=p0.y-p.y;
	double startAngle=atan2(dy0, dx0);
	double dx1=p1.x-p.x;
	double dy1=p1.y-p.y;
	double endAngle=atan2(dy1, dx1);
	if (direction==CGAlgorithms::CLOCKWISE) {
		if (startAngle<=endAngle) startAngle+=2.0*PI;
	} else {    // direction==COUNTERCLOCKWISE
		if (startAngle>=endAngle) startAngle-=2.0*PI;
	}
	addPt(p0);
	addFillet(p, startAngle, endAngle, direction, distance);
	addPt(p1);
}

/**
*add points for a fillet angle.  The start and end point for the fillet are not added -
*it is assumed that the caller will add them
*
*@param direction is -1 for a CW angle, 1 for a CCW angle
*/
void BufferLineBuilder::addFillet(Coordinate &p, double startAngle, double endAngle, int direction, double distance){
	int directionFactor=direction<0 ? -1 : 1;
	double totalAngle=fabs(startAngle-endAngle);
	int nSegs=(int)(totalAngle/angleInc+0.5);
	if (nSegs<1) return;    // no segments because angle is less than increment-nothing to do!
	double initAngle, currAngleInc;
	// choose initAngle so that the segments at each end of the fillet are equal length
	//initAngle=(totalAngle-(nSegs-1)*angleInc) / 2;
	// currAngleInc=angleInc;

	// choose angle increment so that each segment has equal length
	initAngle=0.0;
	currAngleInc=totalAngle/nSegs;
	double currAngle=initAngle;
	Coordinate pt;
	while (currAngle<totalAngle) {
		double angle=startAngle+directionFactor*currAngle;
		pt.x=p.x+distance*cos(angle);
		pt.y=p.y+distance*sin(angle);
		addPt(pt);
		currAngle += currAngleInc;
	}
}

/**
*Adds a CW circle around a point
*/
void BufferLineBuilder::addCircle(Coordinate &p, double distance){
// add start point
	Coordinate& pt=*(new Coordinate(p.x+distance,p.y));
	addPt(pt);
	addFillet(p,0.0,2.0*PI,-1,distance);
}
}

