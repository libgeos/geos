#include "../../headers/indexChain.h"
#include "../../headers/graph.h"
#include "stdio.h"

namespace geos {

/**
* Return a list of the monotone chains
* for the given list of coordinates.
*/
vector<indexMonotoneChain*>* MonotoneChainBuilder::getChains(CoordinateList *pts) {
	vector<indexMonotoneChain*> *mcList=new vector<indexMonotoneChain*>();
	vector<int> *startIndex=getChainStartIndices(pts);
	for(int i=0;i<(int)startIndex->size()-1;i++) {
		indexMonotoneChain *mc=new indexMonotoneChain(pts,(*startIndex)[i],(*startIndex)[i + 1]);
		mcList->push_back(mc);
	}
	delete startIndex;
	return mcList;
}

/**
* Return an array containing lists of start/end indexes of the monotone chains
* for the given list of coordinates.
* The last entry in the array points to the end point of the point array,
* for use as a sentinel.
*/
vector<int>* MonotoneChainBuilder::getChainStartIndices(CoordinateList *pts) {
	// find the startpoint (and endpoints) of all monotone chains in this edge
	int start=0;
	vector<int> *startIndexList=new vector<int>();
	startIndexList->push_back(start);
	do {
		int last=findChainEnd(pts,start);
		startIndexList->push_back(last);
		start=last;
	} while(start<pts->getSize()-1);
	// copy list to an array of ints, for efficiency
	//int[] startIndex = toIntArray(startIndexList);
	return startIndexList;
}

/**
* @return the index of the last point in the monotone chain starting at <code>start</code>.
*/
int MonotoneChainBuilder::findChainEnd(CoordinateList *pts,int start) {
	// determine quadrant for chain
	int chainQuad=Quadrant::quadrant(pts->getAt(start),pts->getAt(start + 1));
	int last=start+1;
	while (last<pts->getSize()) {
		// compute quadrant for next possible segment in chain
		int quad=Quadrant::quadrant(pts->getAt(last-1),pts->getAt(last));
		if (quad!=chainQuad) break;
		last++;
	}
	return last-1;
}
}