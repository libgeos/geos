#include "../headers/geosAlgorithm.h"

namespace geos {

SimplePointInRing::SimplePointInRing(LinearRing *ring){
	pts=ring->getCoordinates();
	cga=new RobustCGAlgorithms();
}

SimplePointInRing::~SimplePointInRing(){
	delete cga;
}

bool SimplePointInRing::isInside(Coordinate& pt){
	return cga->isPointInRing(pt,pts);
}

}

