#include "geom.h"
#include "stdio.h"

bool CoordinateList::hasRepeatedPoints(){
	int size=(int) getSize();
	for(int i=1; i<size; i++) {
		if (getAt(i-1)==getAt(i)) {
			return true;
		}
	}
	return false;
}

Coordinate* CoordinateList::minCoordinate(){
	Coordinate* minCoord=NULL;
	int size=(int) getSize();
	for(int i=0; i<size; i++) {
		if(minCoord==NULL || minCoord->compareTo(getAt(i))>0) {
			minCoord=&getAt(i);
		}
	}
	return minCoord;
}

