#include "geosAlgorithm.h"

CGAlgorithms* SimplePointInRing::cga=new RobustCGAlgorithms();

SimplePointInRing::SimplePointInRing(LinearRing *ring){
	pts=ring->getCoordinates();
}

bool SimplePointInRing::isInside(Coordinate pt){
	return cga->isPointInRing(pt,pts);
}

