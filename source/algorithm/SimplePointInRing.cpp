/*
* $Log$
* Revision 1.12  2003/10/16 08:50:00  strk
* Memory leak fixes. Improved performance by mean of more calls to new getCoordinatesRO() when applicable.
*
*/
#include "../headers/geosAlgorithm.h"

namespace geos {

SimplePointInRing::SimplePointInRing(LinearRing *ring){
	pts=ring->getCoordinatesRO();
	cga=new RobustCGAlgorithms();
}

SimplePointInRing::~SimplePointInRing(){
	delete cga;
}

bool SimplePointInRing::isInside(const Coordinate& pt){
	return cga->isPointInRing(pt,pts);
}

}

