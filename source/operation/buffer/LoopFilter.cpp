#include "../../headers/opBuffer.h"
#include "../../headers/util.h"

namespace geos {

LoopFilter::LoopFilter() {
	newPts=CoordinateListFactory::internalFactory->createCoordinateList();
	maxPointsInLoop=10;           // maximum number of points in a loop
	maxLoopExtent=10.0;    // the maximum X and Y extents of a loop
}

LoopFilter::~LoopFilter() {
	delete newPts;
}

CoordinateList* LoopFilter::filter(CoordinateList *inputPts) {
	delete newPts;
	newPts=CoordinateListFactory::internalFactory->createCoordinateList();
	int i=0;
	while(i<inputPts->getSize()) {
		addPoint(inputPts->getAt(i));
		int loopSize=checkForLoop(inputPts,i);
		// skip loop if one was found
		i++;
		if (loopSize>0) {
			Assert::isTrue(inputPts->getAt(i-1)==inputPts->getAt(i-1+loopSize),"non-loop found in LoopFilter");
			i+=loopSize;
		}
	}
	return newPts;
}

void LoopFilter::addPoint(Coordinate p) {
	// don't add duplicate points
	if (newPts->getSize()>=1 && newPts->getAt(newPts->getSize()-1)==p)
		return;
	newPts->add(p);
}

/**
* Find a small loop starting at this point, if one exists.
* If found, return the index of the last point of the loop.
* If none exists, return 0
*/
int LoopFilter::checkForLoop(CoordinateList *pts,int startIndex){
	Coordinate& startPt=pts->getAt(startIndex);
	Envelope *env=new Envelope();
	env->expandToInclude(startPt);
	int endIndex=startIndex;
	for (int j=1;j<=maxPointsInLoop;j++) {
		endIndex=startIndex+j;
		if (endIndex>=pts->getSize()) break;
		env->expandToInclude(pts->getAt(endIndex));
		if (pts->getAt(endIndex)==startPt) {
			if (env->getHeight()<maxLoopExtent && env->getWidth()<maxLoopExtent) {
				return j;
			}
		}
	}
	return 0;
}
}

